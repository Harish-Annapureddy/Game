//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Functionality to render a glowing outline around client renderable objects.
//
//===============================================================================

#include "cbase.h"
#include "glow_outline_effect.h"
#include "model_types.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "view_shared.h"
#include "viewpostprocess.h"
#include "clienteffectprecachesystem.h"

#define FULL_FRAME_TEXTURE "_rt_FullFrameFB"

ConVar mat_glow_outline_enable( "mat_glow_outline_enable", "1", FCVAR_ARCHIVE, "Enable entity outline glow effects." );
ConVar mat_glow_outline_width( "mat_glow_outline_width", "6.0f", FCVAR_CHEAT, "Width of glow outline effect in screen space." );

extern bool g_bDumpRenderTargets; // in viewpostprocess.cpp

CGlowObjectManager g_GlowObjectManager;

struct ShaderStencilState_t
{
	bool m_bEnable;
	StencilOperation_t m_FailOp;
	StencilOperation_t m_ZFailOp;
	StencilOperation_t m_PassOp;
	StencilComparisonFunction_t m_CompareFunc;
	int m_nReferenceValue;
	uint32 m_nTestMask;
	uint32 m_nWriteMask;

	ShaderStencilState_t()
	{
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}

	void SetStencilState( CMatRenderContextPtr &pRenderContext  )
	{
		pRenderContext->SetStencilEnable( m_bEnable );
		pRenderContext->SetStencilFailOperation( m_FailOp );
		pRenderContext->SetStencilZFailOperation( m_ZFailOp );
		pRenderContext->SetStencilPassOperation( m_PassOp );
		pRenderContext->SetStencilCompareFunction( m_CompareFunc );
		pRenderContext->SetStencilReferenceValue( m_nReferenceValue );
		pRenderContext->SetStencilTestMask( m_nTestMask );
		pRenderContext->SetStencilWriteMask( m_nWriteMask );
	}
};

CLIENTEFFECT_REGISTER_BEGIN(PrecachePostProcessingEffectsGlow)
CLIENTEFFECT_MATERIAL("dev/glow_color")
CLIENTEFFECT_MATERIAL("dev/glow_downsample")
CLIENTEFFECT_MATERIAL("dev/glow_blur_x")
CLIENTEFFECT_MATERIAL("dev/glow_blur_y")
CLIENTEFFECT_MATERIAL("dev/halo_add_to_screen")
CLIENTEFFECT_REGISTER_END()

void CGlowObjectManager::RenderGlowEffects( const CViewSetup *pSetup )
{
	if ( g_pMaterialSystemHardwareConfig->SupportsPixelShaders_2_0() )
	{
		if ( mat_glow_outline_enable.GetBool() )
		{
			CMatRenderContextPtr pRenderContext( materials );

			PIXEvent _pixEvent( pRenderContext, "EntityGlowEffects" );
			ApplyEntityGlowEffects( pSetup, pRenderContext, mat_glow_outline_width.GetFloat() );
		}
	}
}

void CGlowObjectManager::RenderGlowModels( const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext )
{
	//==========================================================================================//
	// This renders solid pixels with the correct coloring for each object that needs the glow.	//
	// After this function returns, this image will then be blurred and added into the frame	//
	// buffer with the objects stenciled out.													//
	//==========================================================================================//
	pRenderContext->PushRenderTargetAndViewport();

	// Save modulation color and blend
	Vector vOrigColor;
	render->GetColorModulation( vOrigColor.Base() );
	float flOrigBlend = render->GetBlend();

	// Get pointer to FullFrameFB
	ITexture *pRtFullFrame = NULL;
	pRtFullFrame = materials->FindTexture( FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET );

	SetRenderTargetAndViewPort( pRtFullFrame );

	pRenderContext->ClearColor3ub( 0, 0, 0 );
	pRenderContext->ClearBuffers( true, false, false );

	// Set override material for glow color
	IMaterial *pMatGlowColor = NULL;

	pMatGlowColor = materials->FindMaterial( "dev/glow_color", TEXTURE_GROUP_OTHER, true );
	g_pStudioRender->ForcedMaterialOverride( pMatGlowColor );

	ShaderStencilState_t stencilState;
	stencilState.m_bEnable = false;
	stencilState.m_nReferenceValue = 0;
	stencilState.m_nTestMask = 0xFF;
	stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
	stencilState.m_PassOp = STENCILOPERATION_KEEP;
	stencilState.m_FailOp = STENCILOPERATION_KEEP;
	stencilState.m_ZFailOp = STENCILOPERATION_KEEP;

	stencilState.SetStencilState( pRenderContext );

	//==================//
	// Draw the objects //
	//==================//
	for ( int i = 0; i < m_GlowObjectDefinitions.Count(); ++ i )
	{
		if ( m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw() )
			continue;

		render->SetBlend( m_GlowObjectDefinitions[i].m_flGlowAlpha );
		Vector vGlowColor = m_GlowObjectDefinitions[i].m_vGlowColor * m_GlowObjectDefinitions[i].m_flGlowAlpha;
		render->SetColorModulation( &vGlowColor[0] ); // This only sets rgb, not alpha

		m_GlowObjectDefinitions[i].DrawModel();
	}	

	if ( g_bDumpRenderTargets )
	{
		DumpTGAofRenderTarget( pSetup->width, pSetup->height, "GlowModels" );
	}

	g_pStudioRender->ForcedMaterialOverride( NULL );
	render->SetColorModulation( vOrigColor.Base() );
	render->SetBlend( flOrigBlend );
	
	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	stencilStateDisable.SetStencilState( pRenderContext );

	pRenderContext->PopRenderTargetAndViewport();
}

void CGlowObjectManager::DownSampleAndBlurRT( const CViewSetup *pSetup, IMatRenderContext *pRenderContext, float flBloomScale, ITexture *pRtFullFrame, ITexture *pRtQuarterSize0, ITexture *pRtQuarterSize1 )
{
	static bool s_bFirstPass = true;

	//===================================
	// Setup state for downsample/bloom
	//===================================

	pRenderContext->PushRenderTargetAndViewport();

	// Get viewport
	int nSrcWidth = pSetup->width;
	int nSrcHeight = pSetup->height;
	int nViewportX, nViewportY, nViewportWidth, nViewportHeight;
	pRenderContext->GetViewport( nViewportX, nViewportY, nViewportWidth, nViewportHeight );

	// Get material and texture pointers
	IMaterial *pMatDownsample = materials->FindMaterial( "dev/glow_downsample", TEXTURE_GROUP_OTHER, true);
	IMaterial *pMatBlurX = materials->FindMaterial( "dev/glow_blur_x", TEXTURE_GROUP_OTHER, true );
	IMaterial *pMatBlurY = materials->FindMaterial( "dev/glow_blur_y", TEXTURE_GROUP_OTHER, true );

	//============================================
	// Downsample _rt_FullFrameFB to _rt_SmallFB0
	//============================================

	// First clear the full target to black if we're not going to touch every pixel
	if ( ( pRtQuarterSize0->GetActualWidth() != ( pSetup->width / 4 ) ) || ( pRtQuarterSize0->GetActualHeight() != ( pSetup->height / 4 ) ) )
	{
		SetRenderTargetAndViewPort( pRtQuarterSize0 );
		pRenderContext->ClearColor3ub( 0, 0, 0 );
		pRenderContext->ClearBuffers( true, false, false );
	}

	// Set the viewport
	SetRenderTargetAndViewPort( pRtQuarterSize0 );

	IMaterialVar *pbloomexpvar = pMatDownsample->FindVar( "$bloomexp", 0 );
	if ( pbloomexpvar != NULL )
	{
		pbloomexpvar->SetFloatValue( 2.5f );
	}

	IMaterialVar *pbloomsaturationvar = pMatDownsample->FindVar( "$bloomsaturation", 0 );
	if ( pbloomsaturationvar != NULL )
	{
		pbloomsaturationvar->SetFloatValue( 1.0f );
	}

	// note the -2's below. Thats because we are downsampling on each axis and the shader
	// accesses pixels on both sides of the source coord
	int nFullFbWidth = nSrcWidth;
	int nFullFbHeight = nSrcHeight;

	pRenderContext->DrawScreenSpaceRectangle( pMatDownsample, 0, 0, nSrcWidth/4, nSrcHeight/4,
		0, 0, nFullFbWidth - 4, nFullFbHeight - 4,
		pRtFullFrame->GetActualWidth(), pRtFullFrame->GetActualHeight() );

	if ( g_bDumpRenderTargets )
	{
		DumpTGAofRenderTarget( nSrcWidth/4, nSrcHeight/4, "QuarterSizeFB" );
	}

	//============================//
	// Guassian blur x rt0 to rt1 //
	//============================//

	// First clear the full target to black if we're not going to touch every pixel
	if ( s_bFirstPass || ( pRtQuarterSize1->GetActualWidth() != ( pSetup->width / 4 ) ) || ( pRtQuarterSize1->GetActualHeight() != ( pSetup->height / 4 ) ) )
	{
		// On the first render, this viewport may require clearing
		s_bFirstPass = false;
		SetRenderTargetAndViewPort( pRtQuarterSize1 );
		pRenderContext->ClearColor3ub( 0, 0, 0 );
		pRenderContext->ClearBuffers( true, false, false );
	}

	// Set the viewport
	SetRenderTargetAndViewPort( pRtQuarterSize1 );

	pRenderContext->DrawScreenSpaceRectangle( pMatBlurX, 0, 0, nSrcWidth/4, nSrcHeight/4,
		0, 0, nSrcWidth/4-1, nSrcHeight/4-1,
		pRtQuarterSize0->GetActualWidth(), pRtQuarterSize0->GetActualHeight() );

	if ( g_bDumpRenderTargets )
	{
		DumpTGAofRenderTarget( nSrcWidth/4, nSrcHeight/4, "GlowX" );
	}

	//============================//
	// Gaussian blur y rt1 to rt0 //
	//============================//
	SetRenderTargetAndViewPort( pRtQuarterSize0 );
	IMaterialVar *pBloomAmountVar = pMatBlurY->FindVar( "$bloomamount", NULL );
	pBloomAmountVar->SetFloatValue( flBloomScale );
	pRenderContext->DrawScreenSpaceRectangle( pMatBlurY, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
		0, 0, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
		pRtQuarterSize1->GetActualWidth(), pRtQuarterSize1->GetActualHeight() );

	if ( g_bDumpRenderTargets )
	{
		DumpTGAofRenderTarget( nSrcWidth/4, nSrcHeight/4, "GlowYAndBloom" );
	}

	// Pop RT
	pRenderContext->PopRenderTargetAndViewport();
}

void CGlowObjectManager::ApplyEntityGlowEffects( const CViewSetup *pSetup, CMatRenderContextPtr &pRenderContext, float flGlowWidth )
{
	//=======================================================//
	// Render objects into stencil buffer					 //
	//=======================================================//
	// Set override shader to the same simple shader we use to render the glow models
	IMaterial *pMatGlowColor = materials->FindMaterial( "dev/glow_color", TEXTURE_GROUP_OTHER, true );
	g_pStudioRender->ForcedMaterialOverride( pMatGlowColor );

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	float flSavedBlend = render->GetBlend();

	// Set alpha to 0 so we don't touch any color pixels
	render->SetBlend( 0.0f );
	pRenderContext->OverrideDepthEnable( true, false );

	int iNumGlowObjects = 0;

	for ( int i = 0; i < m_GlowObjectDefinitions.Count(); ++ i )
	{
		if ( m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw() )
			continue;

		if ( m_GlowObjectDefinitions[i].m_bRenderWhenOccluded || m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded )
		{
			if ( m_GlowObjectDefinitions[i].m_bRenderWhenOccluded && m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded )
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
				stencilState.m_PassOp = STENCILOPERATION_REPLACE;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState( pRenderContext );

				m_GlowObjectDefinitions[i].DrawModel();
			}
			else if ( m_GlowObjectDefinitions[i].m_bRenderWhenOccluded )
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
				stencilState.m_PassOp = STENCILOPERATION_KEEP;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState( pRenderContext );

				m_GlowObjectDefinitions[i].DrawModel();
			}
			else if ( m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded )
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 2;
				stencilState.m_nTestMask = 0x1;
				stencilState.m_nWriteMask = 0x3;
				stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
				stencilState.m_PassOp = STENCILOPERATION_INCRSAT;
				stencilState.m_FailOp = STENCILOPERATION_KEEP;
				stencilState.m_ZFailOp = STENCILOPERATION_REPLACE;

				stencilState.SetStencilState( pRenderContext );

				m_GlowObjectDefinitions[i].DrawModel();
			}
		}

		iNumGlowObjects++;
	}

	// Need to do a 2nd pass to warm stencil for objects which are rendered only when occluded
	for ( int i = 0; i < m_GlowObjectDefinitions.Count(); ++ i )
	{
		if ( m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw() )
			continue;

		if ( m_GlowObjectDefinitions[i].m_bRenderWhenOccluded && !m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded )
		{
			ShaderStencilState_t stencilState;
			stencilState.m_bEnable = true;
			stencilState.m_nReferenceValue = 2;
			stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
			stencilState.m_PassOp = STENCILOPERATION_REPLACE;
			stencilState.m_FailOp = STENCILOPERATION_KEEP;
			stencilState.m_ZFailOp = STENCILOPERATION_KEEP;
			stencilState.SetStencilState( pRenderContext );

			m_GlowObjectDefinitions[i].DrawModel();
		}
	}

	pRenderContext->OverrideDepthEnable( false, false );
	render->SetBlend( flSavedBlend );
	stencilStateDisable.SetStencilState( pRenderContext );
	g_pStudioRender->ForcedMaterialOverride( NULL );

	// If there aren't any objects to glow, don't do all this other stuff
	// this fixes a bug where if there are glow objects in the list, but none of them are glowing,
	// the whole screen blooms.
	if ( iNumGlowObjects <= 0 )
		return;

	//=============================================
	// Render the glow colors to _rt_FullFrameFB 
	//=============================================
	PIXEvent pixEvent( pRenderContext, "RenderGlowModels" );
	RenderGlowModels( pSetup, pRenderContext );
	
	// Get viewport
	int nSrcWidth = pSetup->width;
	int nSrcHeight = pSetup->height;
	int nViewportX, nViewportY, nViewportWidth, nViewportHeight;
	pRenderContext->GetViewport( nViewportX, nViewportY, nViewportWidth, nViewportHeight );

	// Get material and texture pointers
	ITexture *pRtFullFrame = materials->FindTexture( FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET );
	ITexture *pRtQuarterSize0 = materials->FindTexture( "_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET );
	ITexture *pRtQuarterSize1 = materials->FindTexture( "_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET );
	
	DownSampleAndBlurRT( pSetup, pRenderContext, flGlowWidth, pRtFullFrame, pRtQuarterSize0, pRtQuarterSize1 );

	//=======================================================================================================//
	// At this point, pRtQuarterSize0 is filled with the fully colored glow around everything as solid glowy //
	// blobs. Now we need to stencil out the original objects by only writing pixels that have no            //
	// stencil bits set in the range we care about.                                                          //
	//=======================================================================================================//
	IMaterial *pMatHaloAddToScreen = materials->FindMaterial( "dev/halo_add_to_screen", TEXTURE_GROUP_OTHER, true );

	// Do not fade the glows out at all (weight = 1.0)
	IMaterialVar *pDimVar = pMatHaloAddToScreen->FindVar( "$C0_X", nullptr );
	pDimVar->SetFloatValue( 1.0f );

	float flSampleOffsetX = flGlowWidth * ( 1.0f / nSrcWidth );
	float flSampleOffsetY = flGlowWidth * ( 1.0f / nSrcHeight );

	IMaterialVar *pSampleOffsetX = pMatHaloAddToScreen->FindVar( "$C1_X", nullptr );
	pSampleOffsetX->SetFloatValue( flSampleOffsetX );
	IMaterialVar *pSampleOffsetY = pMatHaloAddToScreen->FindVar( "$C1_Y", nullptr );
	pSampleOffsetY->SetFloatValue( flSampleOffsetY );

	// Set stencil state
	ShaderStencilState_t stencilState;
	stencilState.m_bEnable = true;
	stencilState.m_nWriteMask = 0x0; // We're not changing stencil
	stencilState.m_nTestMask = 0xFF;
	stencilState.m_nReferenceValue = 0x0;
	stencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
	stencilState.m_PassOp = STENCILOPERATION_KEEP;
	stencilState.m_FailOp = STENCILOPERATION_KEEP;
	stencilState.m_ZFailOp = STENCILOPERATION_KEEP;
	stencilState.SetStencilState( pRenderContext );

	// Draw quad
	pRenderContext->DrawScreenSpaceRectangle( pMatHaloAddToScreen, 0, 0, nViewportWidth, nViewportHeight,
		0.0f, -0.5f, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
		pRtQuarterSize1->GetActualWidth(),
		pRtQuarterSize1->GetActualHeight() );

	stencilStateDisable.SetStencilState( pRenderContext );
}

bool CGlowObjectManager::GlowObjectDefinition_t::ShouldDraw()
{
    return m_hEntity.Get() &&
        (m_bRenderWhenOccluded || m_bRenderWhenUnoccluded) &&
        m_hEntity->ShouldDraw() &&
        !m_hEntity->IsDormant();
}

void CGlowObjectManager::GlowObjectDefinition_t::DrawModel()
{
	if ( m_hEntity.Get() )
	{
		m_hEntity->DrawModel(STUDIO_RENDER);

		C_BaseEntity *pAttachment = m_hEntity->FirstMoveChild();

		while ( pAttachment != NULL )
		{
			if ( !g_GlowObjectManager.HasGlowEffect( pAttachment ) && pAttachment->ShouldDraw() )
			{
				pAttachment->DrawModel( STUDIO_RENDER );
			}
			pAttachment = pAttachment->NextMovePeer();
		}
	}
}

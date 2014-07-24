#include <Game.hpp>
#include <System/LinuxWindow.hpp>
#include <Renderer/LinuxRendererOGL3.hpp>
#include <System/LinuxInputManager.hpp>

namespace VoxelDemo
{
	ZED_UINT32 Game::PreInitialise( )
	{
		m_pWindow = new ZED::System::LinuxWindow( );

		if( !m_pWindow )
		{
			zedTrace( "[Voxel Demo::Game::PreInitialise] <ERROR> "
				"Failed to create a new window\n" );
				
			return ZED_FAIL;
		}

		m_pRenderer = new ZED::Renderer::LinuxRendererOGL3( );

		if( !m_pRenderer )
		{
			zedTrace( "[Voxel Demo::Game::PreInitialise] <ERROR> "
				"Failed to create a new renderer\n" );

			return ZED_FAIL;
		}

		m_pInputManager = new ZED::System::LinuxInputManager( );

		if( !m_pInputManager )
		{
			zedTrace( "[Voxel Demo::Game::PreInitialise] <ERROR> "
				"Failed to create new input manager\n" );

			return ZED_FAIL;
		}

		return ZED_OK;
	}
}


#include <Configuration.hpp>
#include <System/Memory.hpp>
#include <System/Debugger.hpp>
#include <System/Window.hpp>
#include <cstring>
#include <string>
#include <sstream>

namespace VoxelDemo
{
	Configuration::Configuration( ) :
		m_X( 0 ),
		m_Y( 0 ),
		m_Width( 0 ),
		m_Height( 0 ),
		m_DisplayNumber( -1 ),
		m_ScreenNumber( -1 ),
		m_pFilePath( ZED_NULL )
	{
	}

	Configuration::~Configuration( )
	{
		zedSafeDeleteArray( m_pFilePath );
	}

	ZED_UINT32 Configuration::Read( const ZED_CHAR8 *p_pFilePath )
	{
		if( p_pFilePath )
		{
			// Absolute path, copy the whole path verbatim
			if( p_pFilePath[ 0 ] == '/' )
			{
			}
			else
			{
				// Relative path, find out how many directories we need to go
				// up by, malformed names will be rejected
				ZED_BOOL DoubleDot = ZED_FALSE;
				ZED_UINT32 DirectoryUpCount = 0;
				ZED_MEMSIZE StartingPoint = 0;

				for( ZED_MEMSIZE i = 0; i < strlen( p_pFilePath ); ++i )
				{
					if( p_pFilePath[ i ] == '.' )
					{
						if( DoubleDot == ZED_TRUE )
						{
							if( p_pFilePath[ ++i ] == '/' )
							{
								++DirectoryUpCount;
								DoubleDot = ZED_FALSE;
								StartingPoint = i + 1;
							}
							else
							{
								// The character had to be a slash, otherwise
								// it will not seek correctly
								return ZED_FAIL;
							}
						}
						else
						{
							DoubleDot = ZED_TRUE;
						}
					}
				}

				std::string FilePath( p_pFilePath );
				FilePath = FilePath.substr( StartingPoint,
					strlen( p_pFilePath ) );

				ZED_CHAR8 *pExecutableDirectory =
					new ZED_CHAR8[ ZED_MAX_PATH ];
				memset( pExecutableDirectory, '\0', ZED_MAX_PATH );
				ZED::System::GetExecutablePath( &pExecutableDirectory,
					ZED_MAX_PATH );
				std::string ConfigurationPath( pExecutableDirectory );

				if( DirectoryUpCount > 0 )
				{
					size_t LastChar = ConfigurationPath.size( );
					size_t PreviousLastChar = ConfigurationPath.size( );

					if( ConfigurationPath.find_last_of( "/" ) ==
						ConfigurationPath.size( )-1 )
					{
						PreviousLastChar = ConfigurationPath.size( ) - 2;
					}

					for( ZED_UINT32 i = 0; i < DirectoryUpCount; ++i )
					{
						LastChar = ConfigurationPath.find_last_of( "/",
							PreviousLastChar );
						PreviousLastChar = LastChar-1;
					}

					ConfigurationPath =
						ConfigurationPath.substr( 0, LastChar );
				}

				if( ConfigurationPath.find_last_of( "/" ) !=
					ConfigurationPath.size( )-1 )
				{
					ConfigurationPath.append( "/" );
				}

				ConfigurationPath.append( FilePath );

				m_pFilePath = new ZED_CHAR8[ ConfigurationPath.size( ) + 1 ];
				memset( m_pFilePath, '\0', ConfigurationPath.size( ) + 1 );
				strncpy( m_pFilePath, ConfigurationPath.c_str( ),
					ConfigurationPath.size( ) );
			}
		}
		else
		{
			ZED_CHAR8 *pExecutableDirectory = new ZED_CHAR8[ ZED_MAX_PATH ];

			memset( pExecutableDirectory, '\0',
				sizeof( ZED_CHAR8 ) * ZED_MAX_PATH );

			ZED::System::GetExecutablePath( &pExecutableDirectory,
				ZED_MAX_PATH );
			std::string DefaultConfigurationPath( pExecutableDirectory );

			DefaultConfigurationPath.append( "game.config" );

			ZED_MEMSIZE FilePathLength = DefaultConfigurationPath.size( );
			m_pFilePath = new ZED_CHAR8[ FilePathLength + 1];
			strncpy( m_pFilePath, DefaultConfigurationPath.c_str( ),
				FilePathLength );
			m_pFilePath[ FilePathLength ] = '\0';

			zedSafeDeleteArray( pExecutableDirectory );
		}

		if( ZED::System::FileExists( m_pFilePath, ZED_FALSE ) )
		{
			ZED::System::NativeFile File;
			if( File.Open( m_pFilePath, ZED::System::FILE_ACCESS_READ ) !=
				ZED_OK )
			{
				return ZED_FAIL;
			}

			if( this->ProcessFile( &File ) != ZED_OK )
			{
				File.Close( );

				return ZED_FAIL;
			}

			File.Close( );
		}
		else
		{
			this->LoadDefaults( );

			return ZED_OK;
		}

		std::vector< std::string >::const_iterator LineIterator =
			m_Lines.begin( );

		std::string CurrentType( "Global" );
		zedTrace( "\n\n" );

		while( LineIterator != m_Lines.end( ) )
		{
			if( ( *LineIterator ).find_first_of( "[" ) == 0 )
			{
				ZED_MEMSIZE CloseBracket =
					( *LineIterator ).find_first_of( "]" );
				if( CloseBracket != std::string::npos )
				{
					CurrentType = ( *LineIterator ).substr( 1,
						CloseBracket - 1 );

					this->TrimWhiteSpace( CurrentType );

					++LineIterator;

					continue;
				}
			}
			ZED_MEMSIZE TokenPosition = ( *LineIterator ).find_first_of( "=" );
			if( TokenPosition != std::string::npos )
			{
				std::string Key = ( *LineIterator ).substr( 0, TokenPosition );
				std::string Value =
					( *LineIterator ).substr( TokenPosition + 1 );

				this->TrimWhiteSpace( Key );
				this->TrimWhiteSpace( Value );

				std::map< std::string, std::string > KeyValueMap;

				KeyValueMap.insert( std::pair< std::string, std::string >(
					Key, Value ) );

				m_TypeParameterValue.insert( std::pair< std::string,
					std::map< std::string, std::string > >(
						CurrentType, KeyValueMap ) );

				this->ProcessStackItem( CurrentType, Key, Value );
			}

			++LineIterator;
		}

		this->LoadDefaults( );

		return ZED_OK;
	}

	ZED_UINT32 Configuration::Write( const ZED_CHAR8 *p_pFilePath )
	{
		ZED_CHAR8 *pFilePath = ZED_NULL;
		ZED_BOOL FilePathAllocated = ZED_FALSE;

		if( p_pFilePath )
		{
			pFilePath = const_cast< ZED_CHAR8 * >( p_pFilePath );
		}
		else if( m_pFilePath )
		{
			pFilePath = m_pFilePath;
		}
		else
		{
			ZED_CHAR8 *pExecutableDirectory = new ZED_CHAR8[ ZED_MAX_PATH ];

			memset( pExecutableDirectory, '\0',
				sizeof( ZED_CHAR8 ) * ZED_MAX_PATH );

			ZED::System::GetExecutablePath( &pExecutableDirectory,
				ZED_MAX_PATH );
			std::string DefaultConfigurationPath( pExecutableDirectory );

			DefaultConfigurationPath.append( "bbb.config" );

			ZED_MEMSIZE FilePathLength = DefaultConfigurationPath.size( );
			pFilePath = new ZED_CHAR8[ FilePathLength + 1];
			strncpy( pFilePath, DefaultConfigurationPath.c_str( ),
				FilePathLength );
			pFilePath[ FilePathLength ] = '\0';

			zedSafeDeleteArray( pExecutableDirectory );

			FilePathAllocated = ZED_TRUE;
		}

		// This is an unfortunately naive way of saving the configuration file
		// A better way would be to do this as a difference in the stored value
		// and the one which is stored, which would also require searching
		// through the file looking for the key->value pair to change
		
		ZED::System::NativeFile File;

		if( File.Open( pFilePath, ZED::System::FILE_ACCESS_WRITE ) !=
			ZED_OK )
		{
			return ZED_FAIL;
		}

		std::stringstream Configuration;
		Configuration.str( "" );
		Configuration << "[Graphics]\n";
		Configuration << "\tWidth = " << m_Width << "\n";
		Configuration << "\tHeight = " << m_Height << "\n";
		Configuration << "\tX Position = " << m_X << "\n";
		Configuration << "\tY Position = " << m_Y << "\n";
		Configuration << "\tDisplay Number = " << m_DisplayNumber << "\n";
		Configuration << "\tScreen Number = " << m_ScreenNumber << "\n";

		ZED_MEMSIZE WrittenStringSize = 0;

		File.WriteString( Configuration.str( ).c_str( ),
			Configuration.str( ).size( ), &WrittenStringSize );

		File.Close( );

		if( FilePathAllocated == ZED_TRUE )
		{
			zedSafeDeleteArray( pFilePath );
		}
	
		return ZED_OK;
	}

	ZED_SINT32 Configuration::GetXPosition( ) const
	{
		return m_X;
	}

	ZED_SINT32 Configuration::GetYPosition( ) const
	{
		return m_Y;
	}

	ZED_UINT32 Configuration::GetWidth( ) const
	{
		return m_Width;
	}

	ZED_UINT32 Configuration::GetHeight( ) const
	{
		return m_Height;
	}

	ZED_SINT32 Configuration::GetDisplayNumber( ) const
	{
		return m_DisplayNumber;
	}

	ZED_SINT32 Configuration::GetScreenNumber( ) const
	{
		return m_ScreenNumber;
	}

	void Configuration::SetXPosition( const ZED_SINT32 p_X )
	{
		m_X = p_X;
	}

	void Configuration::SetYPosition( const ZED_SINT32 p_Y )
	{
		m_Y = p_Y;
	}

	void Configuration::SetWidth( const ZED_UINT32 p_Width )
	{
		m_Width = p_Width;
	}

	void Configuration::SetHeight( const ZED_UINT32 p_Height )
	{
		m_Height = p_Height;
	}

	void Configuration::SetDisplayNumber( const ZED_SINT32 p_DisplayNumber )
	{
		m_DisplayNumber = p_DisplayNumber;
	}

	void Configuration::SetScreenNumber( const ZED_SINT32 p_ScreenNumber )
	{
		m_ScreenNumber = p_ScreenNumber;
	}

	ZED_UINT32 Configuration::ProcessFile( ZED::System::NativeFile *p_pFile )
	{
		ZED_MEMSIZE FileSize = p_pFile->GetSize( );
		const ZED_MEMSIZE BufferLength = 1024;
		ZED_CHAR8 *pFileBuffer = new ZED_CHAR8[ BufferLength ];
		ZED_MEMSIZE ReadLength = 0;
		std::string TempString;
		std::string CurrentLine;
		std::string CarryOver = "";

		while( FileSize > 0 )
		{
			memset( pFileBuffer, '\0', BufferLength * sizeof( ZED_CHAR8 ) );

			if( FileSize > BufferLength )
			{
				p_pFile->ReadByte(
					reinterpret_cast< ZED_BYTE * >( pFileBuffer ),
					BufferLength, &ReadLength );
				FileSize -= BufferLength;
			}
			else
			{
				p_pFile->ReadByte(
					reinterpret_cast< ZED_BYTE * >( pFileBuffer ),
					FileSize, &ReadLength );
				FileSize -= FileSize;
			}

			if( !CarryOver.empty( ) )
			{
				CurrentLine += CarryOver;
				CarryOver.clear( );
			}

			TempString = pFileBuffer;
			TempString.resize( BufferLength );

			ZED_MEMSIZE FirstLineFeed = TempString.find_first_of( "\n" );
			ZED_MEMSIZE LastLineFeed = TempString.find_last_of( "\n" );

			if( ( FirstLineFeed == std::string::npos ) ||
				( LastLineFeed == std::string::npos ) )
			{
				CurrentLine += TempString;
				continue;
			}

			ZED_MEMSIZE CursorPosition = 0;
			ZED_MEMSIZE NextLineFeed = 0;

			while( NextLineFeed != LastLineFeed )
			{
				if( CursorPosition == BufferLength )
				{
					break;
				}

				NextLineFeed = TempString.find_first_of( "\n",
					CursorPosition+1 );
				ZED_MEMSIZE LineLength = NextLineFeed - CursorPosition;
				CurrentLine += TempString.substr(
					CursorPosition, LineLength );
				CursorPosition = NextLineFeed;
				this->TrimWhiteSpace( CurrentLine );
				if( !CurrentLine.empty( ) )
				{
					m_Lines.push_back( CurrentLine );
					CurrentLine.clear( );
				}
			}

			if( LastLineFeed != BufferLength )
			{
				if( LastLineFeed == 0 )
				{
					this->TrimWhiteSpace( CurrentLine );
					m_Lines.push_back( CurrentLine );
					CurrentLine.clear( );
				}

				CarryOver = TempString.substr( LastLineFeed+1,
					BufferLength - LastLineFeed + 1 );
				
				// End of file
				if( FileSize == 0 )
				{
					CurrentLine += CarryOver;
					this->TrimWhiteSpace( CurrentLine );

					if( !CurrentLine.empty( ) )
					{
						m_Lines.push_back( CurrentLine );
					}
				}
			}
		}

		zedSafeDeleteArray( pFileBuffer );

		return ZED_OK;
	}

	ZED_UINT32 Configuration::ProcessStackItem( const std::string &p_Type,
		const std::string &p_Key, const std::string &p_Value )
	{
		if( p_Type.compare( "Graphics" ) == 0 )
		{
			if( p_Key.compare( "Width" ) == 0 )
			{
				m_Width = strtoul( p_Value.c_str( ), ZED_NULL, 10 );
				if( m_Width == 0 )
				{
					return ZED_FAIL;
				}

				return ZED_OK;
			}
			if( p_Key.compare( "Height" ) == 0 )
			{
				m_Height = strtoul( p_Value.c_str( ), ZED_NULL, 10 );
				if( m_Height == 0 )
				{
					return ZED_FAIL;
				}

				return ZED_OK;
			}
			if( p_Key.compare( "X Position") == 0 )
			{
				m_X = strtol( p_Value.c_str( ), ZED_NULL, 10 );
				return ZED_OK;
			}
			if( p_Key.compare( "Y Position" ) == 0 )
			{
				m_Y = strtol( p_Value.c_str( ), ZED_NULL, 10 );
				return ZED_OK;
			}
			if( p_Key.compare( "Display Number" ) == 0 )
			{
				m_DisplayNumber = strtol( p_Value.c_str( ), ZED_NULL, 10 );
				return ZED_OK;
			}
			if( p_Key.compare( "Screen Number" ) == 0 )
			{
				m_ScreenNumber = strtol( p_Value.c_str( ), ZED_NULL, 10 );
				return ZED_OK;
			}
		}

		return ZED_FAIL;
	}

	void Configuration::TrimWhiteSpace( std::string &p_String )
	{
		ZED_MEMSIZE LeadPosition = 0;
		ZED_MEMSIZE TrailPosition = 0;
		ZED_MEMSIZE FirstCharPosition = 0;
		ZED_MEMSIZE LastCharPosition = 0;

		LeadPosition = p_String.find_first_of( " \n\t\r" );
		FirstCharPosition = p_String.find_first_not_of( " \n\t\r" );
	
		// Blank line
		if( FirstCharPosition == std::string::npos )
		{
			p_String = "";
			return;
		}

		if( p_String[ 0 ] == 0x00 )
		{
			p_String = "";
			return;
		}

		if( LeadPosition != std::string::npos )
		{
			if( LeadPosition < FirstCharPosition )
			{
				p_String = p_String.substr( FirstCharPosition,
					p_String.size( ) - LeadPosition );
			}
		}

		TrailPosition = p_String.find_last_of( " \n\t\r" );
		LastCharPosition = p_String.find_last_not_of( " \n\t\r" );

		if( TrailPosition != std::string::npos )
		{
			if( TrailPosition > LastCharPosition )
			{
				p_String = p_String.substr( 0, TrailPosition );
			}
		}
	}

	void Configuration::LoadDefaults( )
	{
		if( m_DisplayNumber == -1 || m_ScreenNumber == -1 )
		{
			m_DisplayNumber = ZED::System::GetCurrentDisplayNumber( );
			m_ScreenNumber = ZED::System::GetCurrentScreenNumber( );
		}

		if( m_Width == 0 || m_Height == 0 )
		{
			ZED::System::SCREEN *pScreens;
			ZED_MEMSIZE ScreenCount;
			ZED::System::EnumerateScreens(
				ZED::System::GetCurrentDisplayNumber( ),
				ZED::System::GetCurrentScreenNumber( ),
				&pScreens, &ScreenCount );

			ZED_UINT32 SmallestWidth = pScreens[ 0 ].Width;
			ZED_UINT32 SmallestHeight = pScreens[ 0 ].Height;
			ZED_UINT32 SmallestResolution = SmallestWidth * SmallestHeight;

			for( ZED_MEMSIZE i = 1; i < ScreenCount; ++i )
			{
				if( ( pScreens[ i ].Width * pScreens[ i ].Height ) <
					SmallestResolution )
				{
					SmallestWidth = pScreens[ i ].Width;
					SmallestHeight = pScreens[ i ].Height;

					SmallestResolution = SmallestWidth * SmallestHeight;
				}
			}

			m_Width = SmallestWidth;
			m_Height = SmallestHeight;

			m_X = ( pScreens[ 0 ].Width / 2 ) - ( SmallestWidth / 2 );
			m_Y = ( pScreens[ 0 ].Height / 2 ) - ( SmallestHeight / 2 );

			zedSafeDeleteArray( pScreens );
		}
	}
}


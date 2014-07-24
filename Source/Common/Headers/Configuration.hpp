#ifndef ___CONFIGURATION_HPP__
#define ___CONFIGURATION_HPP__

#include <System/DataTypes.hpp>
#include <System/NativeFile.hpp>
#include <map>
#include <string>
#include <vector>

namespace VoxelDemo
{
	class Configuration
	{
	public:
		Configuration( );
		~Configuration( );

		ZED_UINT32 Read( const ZED_CHAR8 *p_pFilePath = ZED_NULL );
		ZED_UINT32 Write( const ZED_CHAR8 *p_pFilePath = ZED_NULL );

		ZED_SINT32 GetXPosition( ) const;
		ZED_SINT32 GetYPosition( ) const;
		ZED_UINT32 GetWidth( ) const;
		ZED_UINT32 GetHeight( ) const;
		ZED_SINT32 GetDisplayNumber( ) const;
		ZED_SINT32 GetScreenNumber( ) const;

		void SetXPosition( const ZED_SINT32 p_X );
		void SetYPosition( const ZED_SINT32 p_Y );
		void SetWidth( const ZED_UINT32 p_Width );
		void SetHeight( const ZED_UINT32 p_Height );
		void SetDisplayNumber( const ZED_SINT32 p_DisplayNumber );
		void SetScreenNumber( const ZED_SINT32 p_ScreenNumber );

	private:
		typedef std::multimap< std::string,
			std::map< std::string, std::string > > TypeParameterValueMap;

		ZED_UINT32 ProcessFile( ZED::System::NativeFile *p_pFile );
		ZED_UINT32 ProcessStackItem( const std::string &p_Type,
			const std::string &p_Key, const std::string &p_Value );
		void TrimWhiteSpace( std::string &p_String );
		void LoadDefaults( );

		ZED_SINT32	m_X;
		ZED_SINT32	m_Y;
		ZED_UINT32	m_Width;
		ZED_UINT32	m_Height;
		ZED_SINT32	m_DisplayNumber;
		ZED_SINT32	m_ScreenNumber;

		ZED_CHAR8	*m_pFilePath;

		TypeParameterValueMap	m_TypeParameterValue;

		ZED_BOOL				m_ProcessingType;
		ZED_BOOL				m_ProcessingKey;
		ZED_BOOL				m_ProcessingValue;

		std::vector< std::string >	m_Lines;
	};
}

#endif // ___CONFIGURATION_HPP__


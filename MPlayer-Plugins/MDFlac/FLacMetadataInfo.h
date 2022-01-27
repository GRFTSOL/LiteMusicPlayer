// FLacMetadataInfo.h: interface for the CFLacMetadataInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLACMETADATAINFO_H__81741157_D686_44CB_BD17_C30072D8E5C6__INCLUDED_)
#define AFX_FLACMETADATAINFO_H__81741157_D686_44CB_BD17_C30072D8E5C6__INCLUDED_

int MetadataInfo_get_streaminfo_and_tags(IMediaInput *pInput, FLAC__StreamMetadata **streaminfo, FLAC__StreamMetadata **tags);

#endif // !defined(AFX_FLACMETADATAINFO_H__81741157_D686_44CB_BD17_C30072D8E5C6__INCLUDED_)

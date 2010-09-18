/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSiemensMosaicDicomDiffusionImageHeaderReader.h"

#include "gdcmGlobal.h"
#include "gdcmVersion.h"

#if  GDCM_MAJOR_VERSION >= 2
#define DGDCM2
#endif

#ifndef DGDCM2

#include "gdcm.h"

// relevant Siemens private tags
const gdcm::DictEntry SiemensMosiacParameters( 0x0051, 0x100b, "IS", "1", "Mosiac Matrix Size" );
const gdcm::DictEntry SiemensDictNMosiac( 0x0019, 0x100a, "US", "1", "Number of Images In Mosaic" );
const gdcm::DictEntry SiemensDictBValue( 0x0019, 0x100c, "IS", "1", "B Value of diffusion weighting" );
const gdcm::DictEntry SiemensDictDiffusionDirection( 0x0019, 0x100e, "FD", "3", "Diffusion Gradient Direction" );
const gdcm::DictEntry SiemensDictDiffusionMatrix( 0x0019, 0x1027, "FD", "6", "Diffusion Matrix" );
const gdcm::DictEntry SiemensDictShadowInfo( 0x0029, 0x1010, "OB", "1", "Siemens DWI Info" );


#else

#include "gdcmFile.h"
#include "gdcmImageReader.h"
#include "gdcmDict.h"
#include "gdcmDicts.h"
#include "gdcmDictEntry.h"
#include "gdcmDictEntry.h"
#include "gdcmDict.h"
#include "gdcmFile.h"
#include "gdcmSerieHelper.h"

#endif


mitk::SiemensMosaicDicomDiffusionImageHeaderReader::SiemensMosaicDicomDiffusionImageHeaderReader()
{
}

mitk::SiemensMosaicDicomDiffusionImageHeaderReader::~SiemensMosaicDicomDiffusionImageHeaderReader()
{
}

int mitk::SiemensMosaicDicomDiffusionImageHeaderReader::ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray )
{
  std::string::size_type atPosition = tagString.find( nameString );
  if ( atPosition == std::string::npos)
  {
    return 0;
  }
  else
  {
    std::string infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
    const char * infoAsCharPtr = infoAsString.c_str();

    int vm = *(infoAsCharPtr+64);
    std::string vr = infoAsString.substr( 68, 4 );
    //int syngodt = *(infoAsCharPtr+72);
    //int nItems = *(infoAsCharPtr+76);
    //int localDummy = *(infoAsCharPtr+80);

    int offset = 84;
    for (int k = 0; k < vm; k++)
    {
      int itemLength = *(infoAsCharPtr+offset+4);
      int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
      std::string valueString = infoAsString.substr( offset+16, itemLength );
      valueArray.push_back( atof(valueString.c_str()) );
      offset += 16+strideSize;
    }
    return vm;
  }
}

// do the work
void mitk::SiemensMosaicDicomDiffusionImageHeaderReader::Update()
{

  // check if there are filenames
  if(m_DicomFilenames.size())
  {
    // adapted from namic-sandbox
    // DicomToNrrdConverter.cxx

    VolumeReaderType::DictionaryArrayRawPointer inputDict
        = m_VolumeReader->GetMetaDataDictionaryArray();

#ifndef DGDCM2
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensMosiacParameters.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensMosiacParameters);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictNMosiac.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictNMosiac);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictBValue.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictBValue);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionDirection.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionDirection);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionMatrix.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionMatrix);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictShadowInfo.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictShadowInfo);
#endif

    ReadPublicTags();

    int mMosaic = 0;   // number of raws in each mosaic block;
    int nMosaic = 0;   // number of columns in each mosaic block

    std::cout << "Siemens SliceMosaic......" << std::endl;

    m_SliceOrderIS = false;

    // for siemens mosaic image, figure out mosaic slice order from 0029|1010
    std::string tag;
    tag.clear();
    gdcm::File *header0 = new gdcm::File;

#ifndef DGDCM2

    gdcm::BinEntry* binEntry;

    header0->SetMaxSizeLoadEntry(65536);
    header0->SetFileName( m_DicomFilenames[0] );
    header0->SetLoadMode( gdcm::LD_ALL );
    header0->Load();

    // copy information stored in 0029,1010 into a string for parsing
    gdcm::DocEntry* docEntry = header0->GetFirstEntry();
    while(docEntry)
    {
      if ( docEntry->GetKey() == "0029|1010"  )
      {
        binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
        int binLength = binEntry->GetFullLength();
        tag.resize( binLength );
        uint8_t * tagString = binEntry->GetBinArea();

        for (int n = 0; n < binLength; n++)
        {
          tag[n] = *(tagString+n);
        }
        break;
      }
      docEntry = header0->GetNextEntry();
    }

#else

    gdcm::ImageReader reader;
    reader.SetFileName( m_DicomFilenames[0].c_str() );
    if( !reader.Read() )
    {
      itkExceptionMacro(<< "Cannot read requested file");
    }
    const gdcm::File &f = reader.GetFile();
    const gdcm::DataSet &ds = f.GetDataSet();

    //    gdcm::DataSet ds = header0->GetDataSet();
    gdcm::DataSet::ConstIterator it = ds.Begin();

    // Copy of the header->content
    // copy information stored in 0029,1010 into a string for parsing
    for(; it != ds.End(); ++it)
    {
      const gdcm::DataElement &ref = *it;
      if (ref.GetTag() == gdcm::Tag(0x0029,0x1010)) {
        tag = std::string(ref.GetByteValue()->GetPointer(),ref.GetByteValue()->GetLength());
      }
    }

#endif

    // parse SliceNormalVector from 0029,1010 tag
    std::vector<double> valueArray(0);
    int nItems = ExtractSiemensDiffusionInformation(tag, "SliceNormalVector", valueArray);
    if (nItems != 3)  // did not find enough information
    {
      std::cout << "Warning: Cannot find complete information on SliceNormalVector in 0029|1010\n";
      std::cout << "         Slice order may be wrong.\n";
    }
    else if (valueArray[2] > 0)
    {
      m_SliceOrderIS = true;
    }

    // parse NumberOfImagesInMosaic from 0029,1010 tag
    valueArray.resize(0);
    nItems = ExtractSiemensDiffusionInformation(tag, "NumberOfImagesInMosaic", valueArray);
    if (nItems == 0)  // did not find enough information
    {
      std::cout << "Warning: Cannot find complete information on NumberOfImagesInMosaic in 0029|1010\n";
      std::cout << "         Resulting image may contain empty slices.\n";
    }
    else 
    {
      this->m_Output->nSliceInVolume = static_cast<int>(valueArray[0]);
      mMosaic = static_cast<int> (ceil(sqrt(valueArray[0])));
      nMosaic = mMosaic;
    }
    std::cout << "Mosaic in " << mMosaic << " X " << nMosaic << " blocks (total number of blocks = " << valueArray[0] << ").\n"; 


    ReadPublicTags2();

    int nStride = 1;

    std::cout << "Data in Siemens Mosaic Format\n";
    //nVolume = nSlice;
    //std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << this->m_Output->nSliceInVolume << std::endl;
    nStride = 1;

    for (int k = 0; k < m_nSlice; k += nStride )
    {

      gdcm::File *header0 = new gdcm::File;
#ifndef DGDCM2

      gdcm::BinEntry* binEntry;

      header0->SetMaxSizeLoadEntry(65536);
      header0->SetFileName( m_DicomFilenames[0] );
      header0->SetLoadMode( gdcm::LD_ALL );
      header0->Load();

      // copy information stored in 0029,1010 into a string for parsing
      gdcm::DocEntry* docEntry = header0->GetFirstEntry();
      while(docEntry)
      {
        if ( docEntry->GetKey() == "0029|1010"  )
        {
          binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
          int binLength = binEntry->GetFullLength();
          tag.resize( binLength );
          uint8_t * tagString = binEntry->GetBinArea();

          for (int n = 0; n < binLength; n++)
          {
            tag[n] = *(tagString+n);
          }
          break;
        }
        docEntry = header0->GetNextEntry();
      }

#else

      gdcm::ImageReader reader;
      reader.SetFileName( m_DicomFilenames[0].c_str() );
      if( !reader.Read() )
      {
        itkExceptionMacro(<< "Cannot read requested file");
      }
      const gdcm::File &f = reader.GetFile();
      const gdcm::DataSet &ds = f.GetDataSet();

      //    gdcm::DataSet ds = header0->GetDataSet();
      gdcm::DataSet::ConstIterator it = ds.Begin();

      // Copy of the header->content
      // copy information stored in 0029,1010 into a string for parsing
      for(; it != ds.End(); ++it)
      {
        const gdcm::DataElement &ref = *it;
        if (ref.GetTag() == gdcm::Tag(0x0029,0x1010)) {
          tag = std::string(ref.GetByteValue()->GetPointer(),ref.GetByteValue()->GetLength());
        }
      }

#endif

      // parse B_value from 0029,1010 tag
      std::vector<double> valueArray(0);
      vnl_vector_fixed<double, 3> vect3d;
      int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
      if (nItems != 1 || valueArray[0] == 0)  // did not find enough information
      {
        MITK_INFO << "Reading diffusion info from 0019|100c and 0019|100e tags";
        tag.clear();
        bool success = itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0019|100c", tag );
        if(success)
        {
          this->m_Output->bValue = atof( tag.c_str() );
          tag.clear();
          success = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|100e",  tag);
          if(success)
          {
            memcpy( &vect3d[0], tag.c_str()+0, 8 );
            memcpy( &vect3d[1], tag.c_str()+8, 8 );
            memcpy( &vect3d[2], tag.c_str()+16, 8 );
            vect3d.normalize();
            this->m_Output->DiffusionVector = vect3d;
            TransformGradients();
            MITK_INFO << "BV: " << this->m_Output->bValue;
            MITK_INFO << " GD: " << this->m_Output->DiffusionVector << std::endl;
            continue;
          }
        }
      }
      else
      {
        MITK_INFO << "Reading diffusion info from 0029|1010 tags";
        this->m_Output->bValue = valueArray[0];

        // parse DiffusionGradientDirection from 0029,1010 tag
        valueArray.resize(0);
        nItems = ExtractSiemensDiffusionInformation(tag, "DiffusionGradientDirection", valueArray);
        if (nItems == 3)
        {
          vect3d[0] = valueArray[0];
          vect3d[1] = valueArray[1];
          vect3d[2] = valueArray[2];
          vect3d.normalize();
          this->m_Output->DiffusionVector = vect3d;
          TransformGradients();
          MITK_INFO << "BV: " << this->m_Output->bValue;
          MITK_INFO << " GD: " << this->m_Output->DiffusionVector;
          continue;
        }
      }

      MITK_ERROR << "No diffusion info found, assuming BASELINE" << std::endl;
      this->m_Output->bValue = 0.0;
      vect3d.fill( 0.0 );
      this->m_Output->DiffusionVector = vect3d;

    }

  }
}

MITK_CREATE_MODULE(DICOMPM
  DEPENDS MitkCore MitkDICOM MitkModelFit MitkDICOMQI
  PACKAGE_DEPENDS
    PRIVATE DCMTK
)
add_subdirectory(autoload/DICOMPMIO)

coolfluid3_add_library(TARGET coolfluid_ufem_adjoint
  SOURCES
    ActuatorDisk.hpp
    ActuatorDisk.cpp
    ActuatorDisk3D.hpp
    ActuatorDisk3D.cpp
    Adjoint.hpp
    Adjoint.cpp
    keAdjointbase.cpp
    keAdjointbase.hpp
    BCAdjointpressurex.hpp
    BCAdjointpressurex.cpp
    BCAdjointpressurey.hpp
    BCAdjointpressurey.cpp
    BCAdjointpressurez.hpp
    BCAdjointpressurez.cpp
    keAdjoint.cpp
    keAdjoint.hpp
    kaRobinke.cpp
    kaRobinke.hpp
    RobinUt.cpp
    RobinUt.hpp
    LibUFEMAdjoint.hpp
    LibUFEMAdjoint.cpp
  LIBS
    coolfluid_mesh_lagrangep1
    coolfluid_mesh_lagrangep2
    coolfluid_solver_actions
    coolfluid_physics
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)

set_source_files_properties(keAdjoint.cpp PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1024")

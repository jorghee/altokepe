cmake_minimum_required(VERSION 3.16)

project(ManagerEstacionClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Incluir el directorio raíz del proyecto común
include_directories(../../../)

# Definir el ejecutable y sus archivos fuente
add_executable(ManagerEstacionClient
  # Archivo principal
  main.cpp

  # Capa de red
  network/ClienteEstacionApp.cpp
  network/ClienteTCP.cpp

  # Capa de UI
  ui/VentanaEstacion.cpp
  ui/VentanaEstacionesUnificadas.cpp  # ← Agregado

  # Fuentes comunes necesarios
  ../../../common/network/SerializadorJSON.cpp

  # Puedes agregar resources.qrc si lo usas
  # resources.qrc
)

# Vincular las librerías Qt
target_link_libraries(ManagerEstacionClient PRIVATE
  Qt6::Widgets
  Qt6::Network
)

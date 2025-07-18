cmake_minimum_required(VERSION 3.16)

project(ManagerChefClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Añadir el directorio raíz ('altokepe/') a las rutas de inclusión para el compilador.
# Esto permite que #include "common/network/SerializadorJSON.h" funcione.
include_directories(../../../)

# Definir el ejecutable y sus archivos fuente.
# Las rutas deben ser relativas a ESTE archivo CMakeLists.txt
add_executable(ManagerChefClient
  # Archivo principal
  main.cpp
  
  # Recursos de la aplicación
  resources.qrc
  
  # Capa de Red (están en subdirectorios de la ubicación actual)
  network/ClienteManagerApp.cpp
  network/ClienteTCP.cpp

  # Capa de UI (están en subdirectorios de la ubicación actual)
  ui/VentanaManager.cpp
  ui/ColumnaPedidos.cpp
  ui/TarjetaPedido.cpp
  ui/TarjetaPlato.cpp

  # Fuentes del directorio 'common' (necesitan la ruta relativa completa)
  ../../../common/network/SerializadorJSON.cpp
)

# Vincular las librerías de Qt al ejecutable
target_link_libraries(ManagerChefClient PRIVATE
  Qt6::Widgets
  Qt6::Network
)

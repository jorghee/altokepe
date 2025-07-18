cmake_minimum_required(VERSION 3.16)

project(ClienteRecepcionista)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(ClienteRecepcionista
  main.cpp
  resources.qrc

  # Archivos UI
  ui/VentanaRecepcionista.cpp
  ui/VentanaRecepcionista.h
  ui/PanelMesas.cpp
  ui/PanelMesas.h
  ui/PanelPedido.cpp
  ui/PanelPedido.h

  # Archivos de red
  network/ClienteRecepcionista.cpp
  network/ClienteRecepcionista.h 

  # Archivos de lógica de almacenamiento
  data/RegistroPedido.h
  data/ArbolBMas.cpp
  data/ArbolBMas.h
  data/GestorPedidos.cpp
  data/GestorPedidos.h
)

target_link_libraries(ClienteRecepcionista PRIVATE Qt6::Widgets Qt6::Network)

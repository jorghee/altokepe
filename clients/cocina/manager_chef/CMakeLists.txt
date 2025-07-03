cmake_minimum_required(VERSION 3.16)

project(ManagerChefClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Habilitar manejo automático de MOC, UIC, y RCC de Qt
# Esto es esencial para que los signals/slots y otros meta-objetos funcionen
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar los paquetes de Qt6 que necesitamos
# Widgets: para toda la UI
# Network: para la futura conexión con el servidor
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Crear un archivo de recursos para empaquetar el stylesheet
# Esto hace que el archivo .qss se compile dentro del ejecutable
set(RESOURCE_FILES
  resources.qrc
)

# Definir el ejecutable y sus archivos fuente
add_executable(ManagerChefClient
  main.cpp
  ${RESOURCE_FILES}
  ui/VentanaManager.cpp
  ui/ColumnaPedidos.cpp
  ui/TarjetaPedido.cpp
  ui/TarjetaPlato.cpp
)

# Vincular las librerías de Qt al ejecutable
target_link_libraries(ManagerChefClient PRIVATE
  Qt6::Widgets
  Qt6::Network
)

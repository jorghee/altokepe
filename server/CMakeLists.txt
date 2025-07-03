cmake_minimum_required(VERSION 3.16)

project(ServidorAltokepe VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Habilitar manejo automático de MOC, UIC, y RCC de Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Buscar los paquetes de Qt6 que necesitamos para el servidor
find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Añadir la ruta al directorio raíz 'altokepe/' para que los #include funcionen
# Desde 'server/', subimos un nivel (..) para llegar a 'altokepe/'
include_directories(../)

# Definir el ejecutable y todos sus archivos fuente (.cpp)
add_executable(ServidorAltokepe
  main.cpp
  Servidor.cpp
  ManejadorCliente.cpp
  LogicaNegocio.cpp
  
  # Incluimos el archivo .cpp del directorio common que necesita compilación.
  ../common/network/SerializadorJSON.cpp
)

# Vincular las librerías de Qt al ejecutable
target_link_libraries(ServidorAltokepe PRIVATE
  Qt6::Core
  Qt6::Network
)

# Instrucción para copiar menu.json al directorio de compilación
# Esto asegura que el ejecutable pueda encontrarlo al iniciar.
add_custom_command(
  TARGET ServidorAltokepe POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE_DIR:ServidorAltokepe>/../menu.json
  $<TARGET_FILE_DIR:ServidorAltokepe>/menu.json
  COMMENT "Copiando menu.json al directorio de compilación"
)

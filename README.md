# README

## Proyecto: Implementación de la API OSRMS

### **Universidad**: Pontificia Universidad Católica de Chile  
### **Escuela**: Escuela de Ingeniería  
### **Departamento**: Departamento de Ciencia de la Computación  
### **Curso**: IIC2333 — Sistemas Operativos y Redes — 2/2024  
### **Tarea**: Tarea 2  
### **Fecha de Entrega**: Miércoles 16 de Octubre a las 21:00  

---

## **Descripción del Proyecto**

El objetivo de este proyecto es implementar una API que maneje el contenido de una memoria principal simulada mediante un archivo real. La API permite gestionar la memoria de procesos simulados, asignando y liberando memoria como si los procesos estuvieran en ejecución real, y manejando direcciones virtuales y físicas con ayuda de tablas de páginas.

---

## **Funcionalidades Implementadas**

Hemos implementado las siguientes funciones de la API OSRMS:

### **Funciones Generales**
- **`void os_mount(char* memory_path)`**  
  Monta la memoria principal estableciendo la ruta del archivo binario que representa la memoria.

- **`void os_ls_processes()`**  
  Lista los procesos en ejecución presentes en la memoria.

- **`int os_exists(int process_id, char* file_name)`**  
  Verifica si un archivo con el nombre especificado existe en la memoria del proceso dado.

- **`void os_ls_files(int process_id)`**  
  Lista los archivos dentro de la memoria del proceso especificado, mostrando sus nombres y tamaños.

- **`void os_frame_bitmap()`**  
  Imprime el estado actual del Frame Bitmap, incluyendo el conteo de frames ocupados y libres.

- **`void os_tp_bitmap()`**  
  Imprime el estado actual del Bitmap de Tablas de Páginas de Segundo Orden, incluyendo el conteo de tablas ocupadas y libres.

### **Funciones para Procesos**
- **`void os_start_process(int process_id, char* process_name)`**  
  Inicia un proceso con el ID y nombre especificados, actualizando la Tabla de PCBs en la memoria.

- **`void os_finish_process(int process_id)`**  
  Termina el proceso con el ID especificado, liberando toda la memoria asignada y actualizando la Tabla de PCBs.

---

### **Funciones para Archivos**

- **`void os_close(osrmsFile* file_desc)`**  
  Función para cerrar archivos abiertos.
---

## **Funcionalidades No Implementadas**

Lamentablemente, no se implementaron las funciones relacionadas con la gestión de archivos dentro de la memoria. Las siguientes funciones no están disponibles en esta versión de la API:

### **Funciones para Archivos**
- **`osrmsFile* os_open(int process_id, char* file_name, char mode)`**  
  Función para abrir archivos pertenecientes a un proceso.

- **`int os_read_file(osrmsFile* file_desc, char* dest)`**  
  Función para leer archivos desde la memoria montada y copiar su contenido a una ruta local.

- **`int os_write_file(osrmsFile* file_desc, char* src)`**  
  Función para escribir archivos desde una ruta local a la memoria montada.


---

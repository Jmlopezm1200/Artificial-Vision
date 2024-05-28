import cv2
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import os
import threading

# URLs de las cámaras IP
url_camara_izquierda = "http://192.168.43.44:81/stream"
url_camara_derecha = "http://192.168.43.47:81/stream"

# Directorio para guardar las imágenes
directorio_guardado = "imagenes_guardadas"

# Directorios para guardar las imágenes de cada cámara
directorio_izquierda = os.path.join(directorio_guardado, "izquierda")
directorio_derecha = os.path.join(directorio_guardado, "derecha")

# Crear los directorios si no existen
if not os.path.exists(directorio_izquierda):
    os.makedirs(directorio_izquierda)

if not os.path.exists(directorio_derecha):
    os.makedirs(directorio_derecha)

# Función para encontrar el último número de archivo guardado en un directorio
def encontrar_ultimo_numero(directorio):
    archivos = os.listdir(directorio)
    numeros = [int(nombre.split("_")[-1].split(".")[0]) for nombre in archivos if nombre.endswith(".jpg")]
    return max(numeros, default=0)

# Obtener el último número de archivo guardado en los directorios izquierda y derecha
ultimo_numero_izquierda = encontrar_ultimo_numero(directorio_izquierda)
ultimo_numero_derecha = encontrar_ultimo_numero(directorio_derecha)

# Establecer el contador de imágenes para continuar desde el último archivo guardado
contador_imagenes = max(ultimo_numero_izquierda, ultimo_numero_derecha) + 1

# Función para abrir las cámaras IP y mostrar en una sola ventana
def abrir_y_combinar_camaras():
    # Capturar las transmisiones de video de las cámaras IP
    camara_izquierda = cv2.VideoCapture(url_camara_izquierda)
    camara_derecha = cv2.VideoCapture(url_camara_derecha)

    # Verificar si se abrieron correctamente las transmisiones de video
    if not (camara_izquierda.isOpened() and camara_derecha.isOpened()):
        print("Error al abrir una o ambas cámaras IP")
        return

    # Configurar el tamaño de la ventana
    cv2.namedWindow("Cámaras", cv2.WINDOW_NORMAL)

    # Crear una ventana de Tkinter
    root = tk.Toplevel()
    root.title("Cámaras IP")

    # Función para cerrar la ventana cuando se presiona 'q'
    def cerrar_ventana(event):
        root.destroy()

    # Asociar la función de cierre al evento de tecla 'q'
    root.bind('<KeyPress-q>', cerrar_ventana)

    # Loop para mostrar los frames hasta que se presione 'q'
    while True:
        # Capturar frames de ambas cámaras
        ret1, frame_izquierda = camara_izquierda.read()
        ret2, frame_derecha = camara_derecha.read()

        # Verificar si se han capturado correctamente los frames
        if not (ret1 and ret2):
            print("Error al capturar frames de una o ambas cámaras IP")
            break

        # Redimensionar el frame de la cámara derecha para que tenga la misma altura que el de la cámara izquierda
        height, width, _ = frame_izquierda.shape
        frame_derecha = cv2.resize(frame_derecha, (width, height))

        # Combinar frames horizontalmente
        combined_frame = cv2.hconcat([frame_izquierda, frame_derecha])

        # Mostrar el frame combinado
        cv2.imshow("Cámaras", combined_frame)

        # Salir si se presiona la tecla 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Liberar recursos
    camara_izquierda.release()
    camara_derecha.release()
    cv2.destroyAllWindows()
    root.destroy()  # Cerrar la ventana de Tkinter

# Función para abrir las cámaras en un hilo separado
def abrir_camaras_en_hilo():
    thread = threading.Thread(target=abrir_y_combinar_camaras)
    thread.daemon = True
    thread.start()

# Función para guardar una imagen de cada cámara
def guardar_imagenes():
    global contador_imagenes

    # Capturar una imagen de cada cámara
    camara_izquierda = cv2.VideoCapture(url_camara_izquierda)
    camara_derecha = cv2.VideoCapture(url_camara_derecha)

    ret1, frame_izquierda = camara_izquierda.read()
    ret2, frame_derecha = camara_derecha.read()

    if ret1 and ret2:
        # Generar nombres de archivo únicos
        nombre_izquierda = f"imagen_izquierda_{contador_imagenes}.jpg"
        nombre_derecha = f"imagen_derecha_{contador_imagenes}.jpg"

        # Guardar las imágenes en directorios separados
        cv2.imwrite(os.path.join(directorio_izquierda, nombre_izquierda), frame_izquierda)
        cv2.imwrite(os.path.join(directorio_derecha, nombre_derecha), frame_derecha)
        messagebox.showinfo("Guardado", "Imágenes guardadas correctamente")
        
        # Incrementar el contador de imágenes
        contador_imagenes += 1
    else:
        messagebox.showerror("Error", "Error al capturar imágenes")

    # Liberar recursos
    camara_izquierda.release()
    camara_derecha.release()

# Función para guardar imágenes en un hilo separado
def guardar_imagenes_en_hilo():
    thread = threading.Thread(target=guardar_imagenes)
    thread.daemon = True
    thread.start()

# Crear la ventana principal
root = tk.Tk()
root.title("Cámaras IP")

# Obtener las dimensiones de la pantalla
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

# Calcular las nuevas dimensiones y posición para la ventana
new_width = screen_width // 2
new_height = screen_height // 2
x_position = (screen_width - new_width) // 2
y_position = (screen_height - new_height) // 2

# Configurar la geometría de la ventana
root.geometry(f"{new_width}x{new_height}+{x_position}+{y_position}")

# Crear un marco para las cámaras
frame_cameras = tk.Frame(root)
frame_cameras.pack()

# Botón para abrir y combinar cámaras
btn_abrir_camaras = ttk.Button(frame_cameras, text="Abrir Cámaras", command=abrir_camaras_en_hilo)
btn_abrir_camaras.grid(row=0, column=0, padx=10, pady=10)

# Crear un marco para el botón de guardar imágenes
frame_guardar = tk.Frame(root)
frame_guardar.pack()

# Botón para guardar imágenes
btn_guardar_imagenes = ttk.Button(frame_guardar, text="Guardar Imágenes", command=guardar_imagenes_en_hilo)
btn_guardar_imagenes.grid(row=0, column=0, padx=10, pady=10)

# Ejecutar la aplicación
root.mainloop()

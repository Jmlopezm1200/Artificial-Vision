from tkinter import *
import socket

ESP_IP = '192.168.43.57'
ESP_PORT = 8266
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def key_pressed(c):
    print('Key pressed', c.char)
    s.send(c.char.encode(encoding='utf_8'))
def key_released(c):
    print('Key released', c.char)

root = Tk()
root.title("Controlador ESP") 

frame= Frame (root)

lbl_titulo = Label(frame, text="Controlador ESP") 
lbl_titulo.grid(row=0, column=0, pady=10, padx=10)
frame.bind("<KeyPress>", key_pressed)
frame.bind("<KeyRelease>", key_released)

s.connect((ESP_IP , ESP_PORT))

frame.pack()      
frame.focus_set()
root.mainloop()


import pygame
import time
import serial
import threading
import queue
import os
import platform

from conexao import conectar_db

# Conectar ao banco de dados
db, cursor = conectar_db()

def salvar_no_db():
    sql = """
    UPDATE sensores SET
        rpm = %s,
        vel = %s,
        ldr = %s,
        dist = %s,
        farol = %s,
        buzina = %s
    WHERE id_sensor = 1
    """
    valores = (
        dados["rpm"],
        dados["vel"],
        dados["ldr"],
        dados["dist"],
        int(estado["farol"]),
        int(estado["buzina"])
    )
    cursor.execute(sql, valores)
    db.commit()


# PORTA DO HC-05
PORTA = "COM3"
BAUD = 9600

def limpar():
    os.system("cls" if platform.system()=="Windows" else "clear")

def conectar():
    while True:
        try:
            print(f"Tentando conectar Bluetooth em {PORTA}...")
            dev = serial.Serial(PORTA, BAUD, timeout=1)
            time.sleep(2)
            print("Bluetooth conectado!\n")
            return dev
        except:
            print("Falha ao conectar! Tentando de novo...")
            time.sleep(3)

bluetooth = conectar()
fila = queue.Queue()

def leitura():
    while True:
        try:
            if bluetooth.in_waiting:
                linha = bluetooth.readline().decode(errors="ignore").strip()
                if linha:
                    fila.put(linha)
        except:
            pass

threading.Thread(target=leitura, daemon=True).start()

pygame.init()
pygame.joystick.init()
if pygame.joystick.get_count() == 0:
    print("Sem controle.")
    exit()

joy = pygame.joystick.Joystick(0)
joy.init()

estado = {
    "acel": False,
    "re": False,
    "dir": "centro",
    "farol": False,
    "buzina": False
}

dados = {"rpm":0,"vel":0,"ldr":0,"dist":0}

ultimo = ""
ultimo_salvamento = time.time()

def enviar(c):
    global ultimo
    if c != ultimo:
        bluetooth.write((c+"\n").encode())
        ultimo = c

try:
    while True:
        pygame.event.pump()

        eixo = joy.get_axis(0)
        R2 = joy.get_axis(5)
        L2 = joy.get_axis(4)

        estado["acel"] = (R2 > 0.2)
        estado["re"]   = (L2 > 0.2)

        if eixo < -0.3: estado["dir"] = "E"
        elif eixo > 0.3: estado["dir"] = "D"
        else: estado["dir"] = "C"

        for e in pygame.event.get():
            if e.type == pygame.JOYBUTTONDOWN and e.button == 1:
                estado["buzina"] = True
                enviar("B")

            if e.type == pygame.JOYBUTTONUP and e.button == 1:
                estado["buzina"] = False

        if estado["acel"]:
            enviar("F")
        elif estado["re"]:
            enviar("R")
        else:
            if estado["dir"] == "E":
                enviar("E")
            elif estado["dir"] == "D":
                enviar("D")
            else:
                enviar("S")

        while not fila.empty():
            linha = fila.get()

            if "RPM:" in linha:
                dados["rpm"] = float(linha.split(":")[1])

            elif "Velocidade:" in linha:
                dados["vel"] = float(linha.split(":")[1].split()[0])

            elif "LDR:" in linha:
                dados["ldr"] = int(linha.split(":")[1])

            elif "Distancia:" in linha:
                dados["dist"] = int(linha.split(":")[1].split()[0])

        if time.time() - ultimo_salvamento >= 0.2:
            salvar_no_db()
            ultimo_salvamento = time.time()

        limpar()
        print("RPM:", dados["rpm"])
        print("Vel:", dados["vel"])
        print("LDR:", dados["ldr"])
        print("Dist:", dados["dist"])

        time.sleep(0.005)

except KeyboardInterrupt:
    pygame.quit()
    bluetooth.close()
    print("Saindo...")

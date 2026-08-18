import serial
import time
import os
import csv
from datetime import datetime

# =====================================================
# CONFIGURAÇÕES DA PORTA SERIAL
# =====================================================
# Muda isto para a porta onde o teu ESP32-S3 está ligado
PORTA_SERIAL = '/dev/ttyACM0'  # No Windows costuma ser 'COM3', 'COM4', etc.
BAUD_RATE = 115200

# =====================================================
# CONFIGURAÇÕES DA GRAVAÇÃO
# =====================================================
# Tempo que o script vai ficar a gravar o gesto (em segundos)
TEMPO_GRAVACAO = 3.0  

PASTA_DATASET = "dataset"
CABEÇALHO_CSV = [
    "timestamp", "id_luva", 
    "flex_polegar", "flex_indicador", "flex_medio", "flex_anelar", "flex_minimo",
    "acel_x", "acel_y", "acel_z", 
    "giro_x", "giro_y", "giro_z", 
    "roll", "pitch"
]

def inicializar_pasta():
    if not os.path.exists(PASTA_DATASET):
        os.makedirs(PASTA_DATASET)
        print(f"[!] Pasta '{PASTA_DATASET}' criada com sucesso.")

def conectar_serial():
    try:
        ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
        # Dá tempo para o ESP32 fazer o reset ao abrir a porta
        time.sleep(2) 
        ser.reset_input_buffer()
        print(f"[+] Conectado ao ESP32-S3 na porta {PORTA_SERIAL} a {BAUD_RATE} bps.")
        return ser
    except serial.SerialException as e:
        print(f"[-] ERRO: Não foi possível conectar à porta {PORTA_SERIAL}.")
        print(f"    Detalhe: {e}")
        print("    Certifica-te que fechaste o Monitor Serial do VS Code/ESP-IDF!")
        exit(1)

def gravar_gesto(ser, nome_gesto):
    # Cria o nome do ficheiro com timestamp para não sobrescrever gravações antigas
    timestamp_arquivo = datetime.now().strftime("%Y%m%d_%H%M%S")
    nome_ficheiro = f"{PASTA_DATASET}/{nome_gesto}_{timestamp_arquivo}.csv"

    # Envia o comando para o ESP32 entrar no modo de coleta
    print(f"\n[>] A pedir ao ESP32 para entrar em MODO COLETA...")
    ser.write(b"COLETA\n")
    time.sleep(0.5)
    ser.reset_input_buffer()

    print(f"\n=======================================================")
    print(f" 🔴 GRAVANDO GESTO: '{nome_gesto}'")
    print(f" Mantenha o gesto por {TEMPO_GRAVACAO} segundos...")
    print(f"=======================================================")

    amostras_capturadas = 0
    tempo_inicio = time.time()

    with open(nome_ficheiro, mode='w', newline='') as ficheiro_csv:
        escritor = csv.writer(ficheiro_csv)
        escritor.writerow(CABEÇALHO_CSV)

        # Loop de gravação durante o TEMPO_GRAVACAO estipulado
        while (time.time() - tempo_inicio) < TEMPO_GRAVACAO:
            if ser.in_waiting > 0:
                # Lê a linha crua do ESP32
                linha_crua = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # O ESP32 manda: DATA,E,1500,2000,1200,...
                if linha_crua.startswith("DATA,"):
                    partes = linha_crua.split(',')
                    
                    # Verifica se o pacote veio completo (1 DATA + 1 ID + 13 valores = 15 itens)
                    if len(partes) == 15:
                        # Adiciona o timestamp local (ms desde o início da gravação)
                        tempo_atual_ms = int((time.time() - tempo_inicio) * 1000)
                        
                        # Monta a linha para o CSV: [timestamp, id, valores_sensores...]
                        linha_csv = [tempo_atual_ms] + partes[1:]
                        escritor.writerow(linha_csv)
                        amostras_capturadas += 1

    # Volta o ESP32 para o modo DEBUG (para parar de cuspir dados)
    ser.write(b"DEBUG\n")
    print(f"\n[⏹] Gravação concluída! {amostras_capturadas} amostras salvas.")
    print(f"[✔] Guardado em: {nome_ficheiro}")

def main():
    print("\n=============================================")
    print("   COLETOR DE DADOS - SIGN TALK (TINY ML)")
    print("=============================================\n")
    
    inicializar_pasta()
    ser = conectar_serial()

    try:
        while True:
            print("\n---------------------------------------------")
            gesto = input("👉 Qual gesto vais gravar agora? (Ex: letra_A, letra_B, repouso) ou 'sair': ")
            
            if gesto.strip().lower() == 'sair':
                break
            
            if gesto.strip() == "":
                print("Nome do gesto não pode ser vazio!")
                continue

            input(f"Faz a posição da '{gesto}' com a mão e pressiona ENTER para começar a gravar...")
            gravar_gesto(ser, gesto)

    except KeyboardInterrupt:
        print("\n\n[!] Coleta interrompida pelo utilizador.")
    finally:
        if 'ser' in locals() and ser.is_open:
            # Garante que deixamos o ESP32 em silêncio ao sair
            ser.write(b"DEBUG\n")
            ser.close()
            print("[x] Porta Serial fechada.")

if __name__ == "__main__":
    main()

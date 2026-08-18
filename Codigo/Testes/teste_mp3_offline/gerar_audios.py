import os
import urllib.request
import urllib.parse
import sys

# Lista do vocabulário Sign Talk (41 itens: 15 frases/palavras + 26 letras do alfabeto)
VOCABULARIO = [
    # Palavras e Frases
    ("ola", "OLA", "Olá!"),
    ("ola_mundo", "OLA MUNDO", "Olá, mundo!"),
    ("sim", "SIM", "Sim."),
    ("nao", "NAO", "Não."),
    ("obrigado", "OBRIGADO", "Muito obrigado!"),
    ("bom_dia", "BOM DIA", "Bom dia!"),
    ("boa_tarde", "BOA TARDE", "Boa tarde!"),
    ("boa_noite", "BOA NOITE", "Boa noite!"),
    ("tudo_bem", "TUDO BEM", "Tudo bem?"),
    ("sign_talk", "SIGN TALK", "Sign Talk, tradutor de Libras em voz."),
    ("ajuda", "AJUDA", "Preciso de ajuda, por favor."),
    ("por_favor", "POR FAVOR", "Por favor."),
    ("agua", "AGUA", "Quero água."),
    ("paz", "PAZ", "Paz."),
    ("amor", "AMOR", "Amor."),
    
    # Alfabeto (Letras A a Z em Libras)
    ("letra_a", "A", "A"),
    ("letra_b", "B", "B"),
    ("letra_c", "C", "C"),
    ("letra_d", "D", "D"),
    ("letra_e", "E", "E"),
    ("letra_f", "F", "F"),
    ("letra_g", "G", "G"),
    ("letra_h", "H", "H"),
    ("letra_i", "I", "I"),
    ("letra_j", "J", "J"),
    ("letra_k", "K", "K"),
    ("letra_l", "L", "L"),
    ("letra_m", "M", "M"),
    ("letra_n", "N", "N"),
    ("letra_o", "O", "O"),
    ("letra_p", "P", "P"),
    ("letra_q", "Q", "Q"),
    ("letra_r", "R", "R"),
    ("letra_s", "S", "S"),
    ("letra_t", "T", "T"),
    ("letra_u", "U", "U"),
    ("letra_v", "V", "V"),
    ("letra_w", "W", "W"),
    ("letra_x", "X", "X"),
    ("letra_y", "Y", "Y"),
    ("letra_z", "Z", "Z")
]

def baixar_tts_google(texto):
    url = f"http://translate.google.com/translate_tts?ie=UTF-8&client=tw-ob&tl=pt-BR&q={urllib.parse.quote(texto)}"
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)'})
    with urllib.request.urlopen(req) as resp:
        return resp.read()

def main():
    print("==========================================================")
    print("  GERADOR DE ÁUDIOS MP3 OFFLINE (PROGMEM) - SIGN TALK")
    print("==========================================================")
    
    out_dir = os.path.dirname(os.path.abspath(__file__))
    h_path = os.path.join(out_dir, "audios_libras.h")
    
    with open(h_path, "w", encoding="utf-8") as f:
        f.write("/*\n")
        f.write(" * ============================================================================\n")
        f.write(" *  SIGN TALK - VETORES DE ÁUDIO MP3 EM PROGMEM (100% OFFLINE / SEM WI-FI)\n")
        f.write(" *  Arquivo gerado automaticamente pelo script gerar_audios.py\n")
        f.write(" * ============================================================================\n")
        f.write(" */\n\n")
        f.write("#ifndef AUDIOS_LIBRAS_H\n")
        f.write("#define AUDIOS_LIBRAS_H\n\n")
        f.write("#include <Arduino.h>\n\n")
        
        total_bytes = 0
        
        for id_var, comando, fala in VOCABULARIO:
            print(f"[{comando}] Baixando áudio: \"{fala}\"...")
            try:
                mp3_data = baixar_tts_google(fala)
                tamanho = len(mp3_data)
                total_bytes += tamanho
                print(f"  -> Concluído! {tamanho} bytes.")
                
                # Escreve o vetor em PROGMEM
                f.write(f"// Áudio para o comando: \"{comando}\" (Texto falado: \"{fala}\") - {tamanho} bytes\n")
                f.write(f"const unsigned char audio_{id_var}[] PROGMEM = {{\n  ")
                
                # Formata os bytes em hexadecimal (16 por linha)
                hex_bytes = [f"0x{b:02X}" for b in mp3_data]
                lines = [", ".join(hex_bytes[i:i+16]) for i in range(0, len(hex_bytes), 16)]
                f.write(",\n  ".join(lines))
                f.write("\n};\n")
                f.write(f"const unsigned int audio_{id_var}_len = {tamanho};\n\n")
            except Exception as e:
                print(f"  [ERRO] Falha ao baixar \"{fala}\": {e}")
                sys.exit(1)
                
        # Escreve a função de busca
        f.write("// ============================================================\n")
        f.write("// FUNÇÃO DE BUSCA DE ÁUDIO POR COMANDO\n")
        f.write("// ============================================================\n")
        f.write("bool obterAudioLibras(String comando, const unsigned char **audioPtr, unsigned int *audioLen, String *textoExibicao) {\n")
        f.write("  String cmd = comando;\n")
        f.write("  cmd.trim();\n")
        f.write("  cmd.toUpperCase();\n\n")
        
        for id_var, comando, fala in VOCABULARIO:
            f.write(f"  if (cmd == \"{comando}\" || cmd == \"{comando.replace(' ', '')}\" || cmd == \"/{comando}\") {{\n")
            f.write(f"    *audioPtr = audio_{id_var};\n")
            f.write(f"    *audioLen = audio_{id_var}_len;\n")
            f.write(f"    *textoExibicao = \"{fala}\";\n")
            f.write("    return true;\n")
            f.write("  }\n")
            
        f.write("  return false;\n")
        f.write("}\n\n")
        f.write("#endif // AUDIOS_LIBRAS_H\n")
        
    print("==========================================================")
    print(f"[SUCESSO] Arquivo audios_libras.h gerado com {len(VOCABULARIO)} palavras/letras!")
    print(f"Tamanho total ocupado na memória Flash: {total_bytes} bytes ({total_bytes/1024:.1f} KB / {total_bytes/1024/1024:.2f} MB).")
    print("==========================================================")

if __name__ == "__main__":
    main()

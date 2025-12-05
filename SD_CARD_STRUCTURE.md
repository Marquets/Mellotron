# Estructura de la Tarjeta SD para el Mellotron Clone

La tarjeta SD debe estar formateada en **FAT32** o **ExFAT**.

## Estructura de Carpetas

En la raíz de la tarjeta, crea una carpeta llamada `Mellotron`. Dentro de ella, crea una carpeta para cada instrumento (Sonido).

```text
/ (Raíz de la SD)
└── Mellotron/
    ├── Flute/
    │   ├── 36.wav  (Do 2 - C2)
    │   ├── 37.wav  (Do# 2)
    │   ├── ...
    │   └── 71.wav  (Si 4)
    │
    ├── Violins/
    │   ├── 36.wav
    │   ├── ...
    │   └── 71.wav
    │
    ├── Cello/
    │   └── ...
    │
    └── Choir/
        └── ...
```

## Reglas para los Archivos de Audio

1.  **Formato:** WAV
2.  **Resolución:** 16-bit o 24-bit.
3.  **Frecuencia de Muestreo:** 48kHz (Recomendado) o 44.1kHz.
4.  **Canales:** Mono (Recomendado para ahorrar RAM) o Stereo.
5.  **Nombres de Archivo:** Deben ser el **número de nota MIDI**.
    *   El Mellotron original tiene 35 teclas (G2 a F5).
    *   Para simplificar, mapeamos desde C2 (Nota MIDI 36) hasta B4 (Nota MIDI 71).
    *   Ejemplo: La nota Do central (C3) es el archivo `60.wav`.

## Notas Importantes
*   Asegúrate de que los nombres de archivo no tengan espacios ni caracteres extraños.
*   El código buscará exactamente estos números. Si falta un archivo, esa tecla no sonará.

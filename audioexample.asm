#offset 0x00

; --- Program to test PC Speaker Sound ---

start:
    audio.init           ; Initialize audio system

    audio.speaker_on     ; Turn speaker ON

    ; --- Play first tone (440Hz - A4) ---
    mov R0, 440         ; Set register R0 to 440 (frequency in Hz)
    audio.set_pitch R0   ; Set the pitch using R0
    mov R1, 500         ; Set register R1 to 500 (duration in milliseconds)
    sys.wait R1          ; Wait for 500 milliseconds (play tone)

    ; --- Short pause ---
    mov R1, 250         ; Set register R1 to 250 (pause duration)
    audio.speaker_off    ; Turn speaker OFF during pause (silence)
    sys.wait R1          ; Wait for 250 milliseconds (pause)
    audio.speaker_on     ; Turn speaker back ON

    ; --- Play second tone (880Hz - A5 - one octave higher) ---
    mov R0, 880         ; Set register R0 to 880 (frequency in Hz)
    audio.set_pitch R0   ; Set the pitch using R0
    mov R1, 500         ; Set register R1 to 500 (duration in milliseconds)
    sys.wait R1          ; Wait for 500 milliseconds (play tone)

    audio.speaker_off    ; Turn speaker OFF

finish:
    audio.close          ; Close audio system
    hlt                 ; Halt execution

#offset 0x00

; --- Dancing ASCII Art Program ---

.STRING frame1_line1 '     O     '
.STRING frame1_line2 '    /|\\    '
.STRING frame1_line3 '    / \\    '
.STRING frame1_line4 '           '

.STRING frame2_line1 '     O     '
.STRING frame2_line2 '    \\|/    '
.STRING frame2_line3 '    / \\    '
.STRING frame2_line4 '           '

.STRING frame3_line1 '     O     '
.STRING frame3_line2 '    /|\\    '
.STRING frame3_line3 '    \\ /    '
.STRING frame3_line4 '           '

.STRING frame4_line1 '     O     '
.STRING frame4_line2 '    \\|/    '
.STRING frame4_line3 '    \\ /    '
.STRING frame4_line4 '           '

.STRING frame5_line1 '    \\O/    '
.STRING frame5_line2 '     |     '
.STRING frame5_line3 '    / \\    '
.STRING frame5_line4 '           '

.STRING frame6_line1 '    \\O/    '
.STRING frame6_line2 '     |     '
.STRING frame6_line3 '    \\ /    '
.STRING frame6_line4 '           '

.STRING frame7_line1 '    \\O/    '
.STRING frame7_line2 '     |     '
.STRING frame7_line3 '    / \\    '
.STRING frame7_line4 '           '

.STRING frame8_line1 '    \\O/    '
.STRING frame8_line2 '     |     '
.STRING frame8_line3 '    / \\    '
.STRING frame8_line4 '           '


start:
    ; --- Frame 1 ---
    sys.clear_screen        ; Clear screen for each frame
    MOV R0, 10              ; Set text color to light blue
    sys.set_text_color R0

    MOV R0, frame1_line1
    sys.print_string R0
    sys.newline

    MOV R0, 12              ; Set color to light red for torso
    sys.set_text_color R0
    MOV R0, frame1_line2
    sys.print_string R0
    sys.newline

    MOV R0, 14              ; Set color to yellow for legs
    sys.set_text_color R0
    MOV R0, frame1_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7               ; Reset to default color for space
    sys.set_text_color R0
    MOV R0, frame1_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150             ; Wait 150 milliseconds
    sys.wait R0

    ; --- Frame 2 ---
    sys.clear_screen
    MOV R0, 11              ; Set text color to cyan
    sys.set_text_color R0

    MOV R0, frame2_line1
    sys.print_string R0
    sys.newline

    MOV R0, 9               ; Set color to blue for torso
    sys.set_text_color R0
    MOV R0, frame2_line2
    sys.print_string R0
    sys.newline

    MOV R0, 10              ; Set color to light green for legs
    sys.set_text_color R0
    MOV R0, frame2_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame2_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 3 ---
    sys.clear_screen
    MOV R0, 13              ; Set text color to magenta
    sys.set_text_color R0

    MOV R0, frame3_line1
    sys.print_string R0
    sys.newline

    MOV R0, 12              ; Set color to light red for torso
    sys.set_text_color R0
    MOV R0, frame3_line2
    sys.print_string R0
    sys.newline

    MOV R0, 11              ; Set color to cyan for legs
    sys.set_text_color R0
    MOV R0, frame3_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame3_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 4 ---
    sys.clear_screen
    MOV R0, 15              ; Set text color to white bright
    sys.set_text_color R0

    MOV R0, frame4_line1
    sys.print_string R0
    sys.newline

    MOV R0, 14              ; Set color to yellow for torso
    sys.set_text_color R0
    MOV R0, frame4_line2
    sys.print_string R0
    sys.newline

    MOV R0, 13              ; Set color to magenta for legs
    sys.set_text_color R0
    MOV R0, frame4_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame4_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 5 ---
    sys.clear_screen
    MOV R0, 4              ; Set text color to red
    sys.set_text_color R0

    MOV R0, frame5_line1
    sys.print_string R0
    sys.newline

    MOV R0, 3              ; Set color to cyan for torso
    sys.set_text_color R0
    MOV R0, frame5_line2
    sys.print_string R0
    sys.newline

    MOV R0, 2              ; Set color to green for legs
    sys.set_text_color R0
    MOV R0, frame5_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame5_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 6 ---
    sys.clear_screen
    MOV R0, 5              ; Set text color to magenta
    sys.set_text_color R0

    MOV R0, frame6_line1
    sys.print_string R0
    sys.newline

    MOV R0, 6              ; Set color to brown for torso
    sys.set_text_color R0
    MOV R0, frame6_line2
    sys.print_string R0
    sys.newline

    MOV R0, 1              ; Set color to blue for legs
    sys.set_text_color R0
    MOV R0, frame6_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame6_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 7 ---
    sys.clear_screen
    MOV R0, 8              ; Set text color to gray
    sys.set_text_color R0

    MOV R0, frame7_line1
    sys.print_string R0
    sys.newline

    MOV R0, 9              ; Set color to light blue for torso
    sys.set_text_color R0
    MOV R0, frame7_line2
    sys.print_string R0
    sys.newline

    MOV R0, 10              ; Set color to light green for legs
    sys.set_text_color R0
    MOV R0, frame7_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame7_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0

    ; --- Frame 8 ---
    sys.clear_screen
    MOV R0, 3              ; Set text color to cyan
    sys.set_text_color R0

    MOV R0, frame8_line1
    sys.print_string R0
    sys.newline

    MOV R0, 2              ; Set color to green for torso
    sys.set_text_color R0
    MOV R0, frame8_line2
    sys.print_string R0
    sys.newline

    MOV R0, 4              ; Set color to red for legs
    sys.set_text_color R0
    MOV R0, frame8_line3
    sys.print_string R0
    sys.newline

    MOV R0, 7
    sys.set_text_color R0
    MOV R0, frame8_line4
    sys.print_string R0
    sys.newline

    MOV R0, 150
    sys.wait R0


    JMP start             ; Loop back to start for continuous animation
    HLT                     ; Halt (though loop will prevent reaching this)

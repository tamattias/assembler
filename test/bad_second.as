; First pass should work OK. Second pass should fail with errors.

; Entry directive without label.
.entry

; Symbol referenced by operand doesn't exist.
add #2, NoSuchLabel

; Entry point with invalid label.
    .entry   NoSuchLabel




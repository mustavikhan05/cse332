.data
arr: .word 3,4,6,10,2,8
size: .word 6
.text
main:
    jal $31,compute_stats
    j end
compute_stats:
    addi $4,$0,0
    lw $5,size
    lw $6,arr
    add $7,$6,$0
    add $8,$0,$0
loop:
    slt $9,$4,$5
    xori $9,$9,1
    beq $9,$0,done
    sll $10,$4,2
    add $11,$10,$0
    addi $11,$11,arr
    lw $12,0($11)
    slt $13,$12,$6
    xori $13,$13,1
    beq $13,$0,skip_min
    add $6,$12,$0
skip_min:
    slt $13,$7,$12
    xori $13,$13,1
    beq $13,$0,skip_max
    add $7,$12,$0
skip_max:
    add $8,$8,$12
    addi $4,$4,1
    j loop
done:
    jr $31
end: 
    j end
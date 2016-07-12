	 move  $t0, $s0
     lw    $t0, 0($at)          # fetch i
     beq   $t1, $zero, def      # no, -> default
     sll   $t0, $t0, 2          # turn i into a byte offset
     add   $t2, $t0, $at
     lw    $t2, 1064($t2)       # fetch the branch table entry
     jr    $t2                  # go...
is0: sw    $zero, 28($at)       # A[0] = 0
     j     done
is1: 
is2: addi  $t0, $zero, 1        # = 1
     sw    $t0, 32($at)         # A[1] = 1
     j     done
def: addi  $t0, $zero, -1       # = -1
     sw    $t0, 28($at)         # A[0] = -1
     j     done
done:
    add    $t0, $at, $zero      # &A[0] - 28
    lw     $t1, 4($at)          # fetch N
    sll    $t1, $t1, 2          # N as byte offset
    add    $t1, $t1, $at        # &A[N] - 28
    ori    $t2, $zero, 256      # MAX_SIZE
top:
    sltu   $t3, $t0, $t1        # have we reached the final address?
    beq    $t3, $zero, done     # yes, we're done
    sw     $t2, 28($t0)         # A[i] = 0
    addi   $t0, $t0, 4          # update $t0 to point to next element
    j      top                  # go to top of loop
done:

# NOTE: We have not updated i in memory!
    lw    $t0, 1060($at)      # fetch background
    sll   $t1, $t1, 2         # times 2
    lui   $t2, 0xffff         # $t2 = 0xffff0000
    ori   $t2, $t2, 0x00ff    # $t2 = 0xffff00ff
    or    $t0, $t0, $t1       # new value
    sw    $t0, 1060($at)      # background = ...
    lw     $t0, 0($at)        # fetch i
    lw     $t1, 4($at)        # fetch N
    slt    $t1, $t0, $t1      # set $t1 to 1 if $t0 < $t1, to 0 otherwise
    beq    $t1, $zero, skip   # branch if result of slt is 0 (i.e., !(i<N))
    sll    $t0, $t0, 2        # i as a byte offset
    add    $t0, $t0, $at      # &A[i] - 28
    sw     $zero, 28($t0)     # A[i] = 0
skip:
# A[i] = A[i/2] + 1;
    lw     $t0, 0($at)       # fetch i
    sll    $t1, $t1, 2       # turn i/2 into a byte offset (*4)
    add    $t1, $at, $t1     # &A[i/2] - 28
    lw     $t1, 28($t1)      # fetch A[i/2]
    addi   $t1, $t1, 1       # A[i/2] + 1
    sll    $t2, $t0, 2       # turn i into a byte offset 
    add    $t2, $t2, $at     # &A[i] - 28
    sw     $t1, 28($t2)      # A[i] = ...
# A[i+1] = -1;
    addi   $t1, $zero, -1    # -1
    sw     $t1, 32($t2)      # A[i+1] = -1
    lw     $t0, 4($at)       # fetch N
    mult   $t0, $t0, $t0     # N*N
    lw     $t1, 4($at)       # fetch N
    ori    $t2, $zero, 3     # 3
    mult   $t1, $t1, $t2     # 3*N
    add    $t2, $t0, $t1     # N*N + 3*N
    sw     $t2, 0($at)       # i = ...
    rem    $t0, $s0, $s1
    lbu    $t0, 3($at)

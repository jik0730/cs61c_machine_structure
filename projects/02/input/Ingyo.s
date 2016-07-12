	 move  $t0, $s0
     lw    $t0, 0($at)          # fetch i
     beq   $t1, $0, def      # no, -> default
     sll   $t0, $t0, 2          # turn i into a byte offset
     addu   $t2, $t0, $at
     lw    $t2, 1064($t2)       # fetch the branch table entry
     jr    $t2                  # go...
is0: sw    $0, 28($at)       # A[0] = 0
     j     done
is1: 
is2: addiu  $t0, $0, 1        # = 1
     sw    $t0, 32($at)         # A[1] = 1
     j     done
def: addiu  $t0, $0, -1       # = -1
     sw    $t0, 28($at)         # A[0] = -1
     j     done
done:
    addu    $t0, $at, $0      # &A[0] - 28
    lw     $t1, 4($at)          # fetch N
    sll    $t1, $t1, 2          # N as byte offset
    addu    $t1, $t1, $at        # &A[N] - 28
    ori    $t2, $0, 256      # MAX_SIZE
top:
    sltu   $t3, $t0, $t1        # have we reached the final address?
    beq    $t3, $0, done     # yes, we're done
    sw     $t2, 28($t0)         # A[i] = 0
    addiu   $t0, $t0, 4          # update $t0 to point to next element
    j      top                  # go to top of loop
done1:

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
    beq    $t1, $0, skip   # branch if result of slt is 0 (i.e., !(i<N))
    sll    $t0, $t0, 2        # i as a byte offset
    addu    $t0, $t0, $at      # &A[i] - 28
    sw     $0, 28($t0)     # A[i] = 0
skip:
# A[i] = A[i/2] + 1;
    lw     $t0, 0($at)       # fetch i
    sll    $t1, $t1, 2       # turn i/2 into a byte offset (*4)
    addu    $t1, $at, $t1     # &A[i/2] - 28
    lw     $t1, 28($t1)      # fetch A[i/2]
    addiu   $t1, $t1, 1       # A[i/2] + 1
    sll    $t2, $t0, 2       # turn i into a byte offset 
    addu    $t2, $t2, $at     # &A[i] - 28
    sw     $t1, 28($t2)      # A[i] = ...
# A[i+1] = -1;
    addiu   $t1, $0, -1    # -1
    sw     $t1, 32($t2)      # A[i+1] = -1
    lw     $t0, 4($at)       # fetch N
    mult   $t0, $t1
    lw     $t1, 4($at)       # fetch N
    ori    $t2, $0, 3     # 3
    mult   $t1, $t1
    addu    $t2, $t0, $t1     # N*N + 3*N
    sw     $t2, 0($at)       # i = ...
    rem    $t0, $s0, $s1
    lbu    $t0, 3($at)

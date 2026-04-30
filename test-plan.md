# ACLIC Test Plan

1. Basic
  - Test source active in M-mode domain
2. Detached
  - Wire input is ignored for detached sources 
3. Delegation
  - Test delegation of interrupt to S-mode domain
  - Sources cannot be delegated below S-mode domain
  - Child index is 0 for delegated sources
  - Clearing delegated bit in M-mode domain makes source appear unimplemented
    in S-mode domain
  - An interrupt source is inactive in the interrupt domain if the source is
    delegated
4. Read-only zero
  - Unimplemented sources have read-only-zero pending, enabled, iprio, and
    sourcecfg
  - Inactive sources have read-only-zero pending, enabled, and iprio
  - First byte of acliciprio0 is read-only zero
  - First two bytes of aclicsourcecfg0 is read-only zero
5. Illegal
  - Odd-numbered xiselect between 0x1000 and 0x1100 do not exist for XLEN=64
  - xireg4-6 cause illegal instruction exception
  - When mstateen0.ACLIC (bit 53) is 0, accessing acliciprio and aclicsourcecfg
    in S-mode or lower raises illegal instruction exception
6. Priority
  - target[i].iprio is WARL: between 1 and (2^IPRIOLEN)-1
  - When interrupt sources have equal priority number, the source with the
    lowest identity number has the highest priority
  - Set/clear multiple pending/endabled bits for sources with different
    priorities in a single write a write
  - TODO: Sources with iprio >= xithreshold & NIPPRIO\_MASK are suppressed
  - TODO: While handling an interrupt with iprio N, interrupt sources with iprio >= N
    are suppressed until handling is finished
  - TODO: Change priority of pending interrupt
  - TODO: xithreshold is saved/restore to/from xpistatus.pithreshold
7. Preemption
  - TODO: Signal interrupt with lower priority during handler
8. Misc
  - M- and S-level domains have distinct sourcecfg, iprio, eie, and eip
  - Pending bit is read-only for level-sensitive sources
  - Transition between different source modes for different source states
  - TODO: Test with and without S-level domain
  - TODO: Use CSR set/clear instructions for things like D bit in sourcecfg
9. Interrupt handler push and pop
  - TODO: store page fault during context save
10. Stack pointer swap
  - PPUSH is saved unconditionally
  - PUSH is restored unconditionally
  - PUSH is cleared on push
  - PPUSH is cleared on pop

.global rdrand
rdrand:
  rdrand %rax
  jnc rdrand
  ret

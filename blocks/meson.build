blocks = [
    ['constellation', [ui_lib]],
    ['gen'],
    ['hilbert'],
    ['inspect'],
    ['stream-filter'],
    ['throttle'],
    ['unwrap'],
    ['wrap'],
]

foreach b : blocks
    if b.length() >= 2
        deps = b[1]
    else
        deps = []
    endif

    executable(b[0], b[0].underscorify() + '.cpp',
               dependencies: [sdr_lib, deps])
endforeach

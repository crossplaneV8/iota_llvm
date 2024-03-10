# iota_llvm
- iota is a statically typed toy language based on LLVM  
- adopts a minimalist design philosophy and runs as fast as C  
- ABI compatible with C functions which in favors of mixed programming  

# code style
*iota adopts s-expression syntax*
```scheme
// short comment
/* long comment */

// declare variable:
(var a:i32 1)                   // int a = 1;
(var b:f64 (+ a 3))             // double b = a + 3;
(var x 0x7fff)                  // auto x = 0x7fff;
(var y 3.1416)                  // auto y = 3.1416;
(var str "hello world")         // auto str = "hello world";
(var vec (array:f32 len))       // float vec[len];
(var ptr:*i32 (malloc 64))      // int *ptr = malloc(64);

// math expression:
(> i 0)                         // (i > 0)
(+ a (* b c))                   // (a + (b * c))

// assign value:
(set a (+ a 3))                 // a = (a + 3);
(set (vec 2) 7)                 // vec[2] = 7;
(++ i)                          // i = i + 1;

// define function
(def (sum vec:*i32 len)         // int64_t sum(int *vec, int64_t len) {
    (var s 0)                   //     int64_t s = 0;
    (var i 0)                   //     int64_t i = 0;
    (while (< i len)            //     while (i < len) {
        (set s (+ s [vec i]))   //         s = s + vec[i];
        (++ i)                  //         i = i + 1;
    )                           //     }
    s                           //     return s;
)                               // }

// import C library function
(import (sqrt:f64 x:f64))
(import (memcpy:i32 dst:*i8 src:*i8 size:i64))
```

# showcase
*fast inverse-square-root using float-point bit hack:*
```scheme
(def (inv_sqrt:f64 x:f64)
    (var magic:i64 0x5fe6ec85e7de30da)
    (var xhalf:f64 (* x 0.5))
    (var i:i64 (bitcast:i64 x))
    (set i (- magic (>> i 1)))
    (var y:f64 (bitcast:f64 i))
    (set y (* y (- 1.5 (* xhalf y y))))
    (set y (* y (- 1.5 (* xhalf y y))))
    (set y (* y (- 1.5 (* xhalf y y))))
    y
)
```
<br>

*BrainFuck interpreter:*
```scheme
(def (brain_fuck code:*i8)
    (var stack (array:i8 65536))
    (var sp 0)
    (var pc 0)

    (while (!= [code pc] 0)
        (var instr [code pc])
        (if (== instr '<') (-- sp))
        (if (== instr '>') (++ sp))
        (if (== instr '+') (set [stack sp] (+ [stack sp] 1)))
        (if (== instr '-') (set [stack sp] (- [stack sp] 1)))
        (if (== instr ',') (set [stack sp] (getchar)))
        (if (== instr '.') (putchar [stack sp]))
        (if (and (== instr '[') (== [stack sp] 0))
            (begin
                (var n 1)
                (while (!= n 0)
                    (++ pc)
                    (if (== [code pc] '[') (++ n))
                    (if (== [code pc] ']') (-- n))
                )
            )
        )
        (if (and (== instr ']') (!= [stack sp] 0))
            (begin
                (var n 1)
                (while (!= n 0)
                    (-- pc)
                    (if (== [code pc] '[') (-- n))
                    (if (== [code pc] ']') (++ n))
                )
            )
        )
        (++ pc)
    )
)
```

# build the compiler
- ubuntu22.04 (recommended)  
- llvm-14 (sudo apt install llvm)  
- clang-14 (sudo apt install clang)  
```bash
cd iota_llvm
python3 build.py
```

# run demo
```bash
cd iota_llvm
./iota_compiler demo/xxxxxx.iota out.ll
lli out.ll
```


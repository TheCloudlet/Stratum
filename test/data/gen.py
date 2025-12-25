import random

def write_header(f):
    f.write("# Type  Addr\n")

# 1. Sequential: Stride = Block Size (0% Hit Rate)
def gen_sequential(filename, count=1000):
    with open(filename, 'w') as f:
        write_header(f)
        addr = 0x1000
        for _ in range(count):
            op = 'S' if random.random() < 0.2 else 'L'
            f.write(f"{op}       0x{addr:X}\n")
            addr += 0x40 # 64 bytes

# 2. Random (Uniform): Spread over 64MB (0% Hit Rate usu.)
def gen_random(filename, count=1000):
    with open(filename, 'w') as f:
        write_header(f)
        for _ in range(count):
            op = 'S' if random.random() < 0.3 else 'L'
            addr = random.randint(0, 0xFFFFF) * 0x40 
            f.write(f"{op}       0x{addr:X}\n")

# 3. Temporal: 5 Hot blocks (Near 100% Hit Rate)
def gen_temporal(filename, count=1000):
    with open(filename, 'w') as f:
        write_header(f)
        working_set = [0x1000 + i * 0x1000 for i in range(5)] 
        for _ in range(count):
            addr = random.choice(working_set)
            f.write(f"L       0x{addr:X}\n")

# 4. Spatial: Sequential words in block (87.5% Hit Rate)
def gen_spatial(filename, count=1000):
    with open(filename, 'w') as f:
        write_header(f)
        base_addr = 0x50000
        for i in range(count):
             offset = (i % 8) * 8 
             block_offset = (i // 8) * 64
             addr = base_addr + block_offset + offset
             f.write(f"L       0x{addr:X}\n")

# 5. Large Loop: 64KB set (Exceeds L1 32KB, hits L2)
def gen_large_loop(filename, count=2000):
    with open(filename, 'w') as f:
        write_header(f)
        start_addr = 0x20000
        # 64KB range / 64B block = 1024 blocks
        num_blocks = 1024 
        
        ops_written = 0
        while ops_written < count:
             cur = start_addr
             for i in range(num_blocks):
                 f.write(f"L       0x{cur:X}\n")
                 cur += 64
                 ops_written += 1
                 if ops_written >= count: break

# 6. Gaussian (Normal Distribution):
# Center = 0x80000 
# Sigma = 1024 blocks (64KB). 
# +/- 1 sigma = 128KB range (Fits in L2 commonly, partial L1)
def gen_gaussian(filename, count=2000):
    with open(filename, 'w') as f:
        write_header(f)
        mean = 0x80000
        # Sigma 64KB approx (1000 blocks * 64)
        sigma = 1000 * 64 
        
        for _ in range(count):
            op = 'S' if random.random() < 0.2 else 'L'
            # Gauss returns float
            val = random.gauss(mean, sigma)
            
            # Align to 64 bytes to make hits possible
            addr = int(val) 
            addr = (addr // 64) * 64
            
            # Ensure positive address
            if addr < 0: addr = 0
            
            f.write(f"{op}       0x{addr:X}\n")

if __name__ == "__main__":
    gen_sequential("sequential.txt", 5000)
    gen_random("random.txt", 5000)
    gen_temporal("temporal.txt", 5000)
    gen_spatial("spatial.txt", 5000)
    gen_large_loop("largeloop.txt", 5000)
    gen_gaussian("gaussian.txt", 5000)

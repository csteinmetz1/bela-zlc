import numpy as np
import matplotlib.pyplot as plt

blocks = [
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
]

direct = [
    15.48,
    17.02,
    24.96,
    50.82,
    157.93,
    601.19,
    2218.42,
    9348.04,
    36895.13,
    154896.50,
]

fft = [
    31.00,
    33.78,
    33.83,
    44.84,
    64.02,
    94.29,
    161.30,
    325.70,
    621.68,
    1029.86,
]

blocks = np.array(blocks)
fft = np.array(fft) * 2
direct = np.array(direct)

logn = np.array(blocks) * np.log10(np.array(blocks))
nsqrd = np.array(blocks) ** 2

fig, ax = plt.subplots(figsize=(5, 3))

ax.plot(blocks, direct, label="Direct")
ax.plot(blocks, fft, label="FFT")
# ax.plot(blocks, logn, label="O(N log N)")
# ax.plot(blocks, nsqrd, label="O(N^2)")

ax.set_xticks([16, 128, 256, 512, 1024])
ax.set_xlim([16, 1024])
ax.set_ylim([0, 40e3])
# ax.set_yscale("log")
ax.grid(c="lightgray")
ax.set_ylabel("Runtime (μs)")
ax.set_xlabel("Filter size")
plt.tight_layout()
plt.legend()
plt.savefig("docs/figures/runtime.pdf")
plt.savefig("docs/figures/runtime.png")

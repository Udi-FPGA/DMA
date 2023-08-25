# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: D:\Udi\DMA\Git\D2508\DMA\Software\DMA_system\_ide\scripts\debugger_dma-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source D:\Udi\DMA\Git\D2508\DMA\Software\DMA_system\_ide\scripts\debugger_dma-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Arty Z7 003017A6FEF8A" && level==0 && jtag_device_ctx=="jsn-Arty Z7-003017A6FEF8A-23727093-0"}
fpga -file D:/Udi/DMA/Git/D2508/DMA/Software/DMA/_ide/bitstream/DMA_BD_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw D:/Udi/DMA/Git/D2508/DMA/Software/DMA_BD_wrapper/export/DMA_BD_wrapper/hw/DMA_BD_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source D:/Udi/DMA/Git/D2508/DMA/Software/DMA/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow D:/Udi/DMA/Git/D2508/DMA/Software/DMA/Debug/DMA.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con

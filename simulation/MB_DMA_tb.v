`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 08/25/2023 06:44:22 PM
// Design Name: 
// Module Name: MB_DMA_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module MB_DMA_tb();
  wire [5:0]rgb_led_tri_o;
    DMA_BD DMA_BD_i
(
        .rgb_led_tri_o(rgb_led_tri_o));
endmodule

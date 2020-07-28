`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06/25/2020 05:43:10 PM
// Design Name: 
// Module Name: top
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


module top(
input clk,
input  reset);

reg [9:0] raddr = 0;
reg [9:0] waddr = 0;
reg  [31:0] din = 0;
reg  [31:0] douta = 0;

(* dont_touch = "true" *) memory #("init.mem", 1, 64, 512) mem(
	.clk(clk), 
	.raddr(raddr), 
	.waddr(waddr), 
	.din(din), 
	.douta(dout), 
	.reset(reset));

endmodule
 
module memory #(
    parameter F_INIT="init.mem",
    parameter INIT_ISHEX = 1,
    //parameter WID_MEM=16,
    //parameter DEPTH_MEM=2048)
    parameter WID_MEM=1,
    parameter DEPTH_MEM=16384)    
    (input clk,
    input [9:0] raddr,
    input [9:0] waddr,
    input [WID_MEM-1:0] din,
    output [WID_MEM-1:0] douta,
    input  reset);
    
    reg [WID_MEM-1:0] dout;
    (* ram_style = "block" *) reg [WID_MEM-1:0] ram [0:DEPTH_MEM-1];
    
    if (INIT_ISHEX)
        initial $readmemh(F_INIT, ram);
        //initial $readmemh ("../init/1_by_16k.txt",ram);
    else
        initial $readmemb(F_INIT,ram);
    
    always @(posedge clk) begin
        dout <= ram[raddr];
        ram[waddr]<= din; 
    end
    
    assign douta = dout;
endmodule

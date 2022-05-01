import chisel3._

import chisel3.util._

class Npc extends Module {
	val io = IO(new Bundle{
		val mem_data = Input(UInt(64.W))
		val mem_addr = Output(UInt(64.W))
		val instr_data = Input(UInt(64.W))
		val instr_addr =  Output(UInt(64.W))
		val mem_op = Output(UInt(3.W)) 
		val mem_we = Output(Bool())
	})
}


class RegisterFile(val bit_size : Int)extends Module{
	val io = IO(new Bundle{
		val r1 = Input(UInt(5.W))
		val r2 = Input(UInt(5.W))
		val rd = Input(UInt(5.W))
		val we = Input(Bool())
		val rd_data = Input(UInt(bit_size.W))
		val r1_data = Output(UInt(bit_size.W))
		val r2_data = Output(UInt(bit_size.W))
	})
	val reg_vec = RegInit(VecInit(Seq.fill(32)(0.U(bit_size.W))))
	io.r1_data := reg_vec(io.r1)
	io.r2_data := reg_vec(io.r2)
	when(io.we){
		reg_vec(io.rd) := Mux(io.rd === 0.U(5.W), 0.U(bit_size.W), io.rd_data)
	}
	
}

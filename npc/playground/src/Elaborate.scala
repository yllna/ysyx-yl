import chisel3._
object Elaborate extends App {
  // (new chisel3.stage.ChiselStage).execute(args, Seq( chisel3.stage.ChiselGeneratorAnnotation( () => new Npc() ) )  )
   (new chisel3.stage.ChiselStage).execute(args, Seq( chisel3.stage.ChiselGeneratorAnnotation( () => new PcRegister(64) ) )  )
}

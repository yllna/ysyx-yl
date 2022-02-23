#include "Vdoublecontrol.h"
#include "verilated_vcd_c.h"
#include "verilated.h"
#include <stdio.h>
#include <assert.h>
#define MAX_SIM_TIME 20

vluint64_t sim_time = 0;

int main(int argc, char ** argv, char ** env){
	VerilatedContext *contextp = new VerilatedContext;
	Verilated :: traceEverOn(true);
	VerilatedVcdC* tfp = new VerilatedVcdC;
	Vdoublecontrol * top = new Vdoublecontrol{contextp};
	top->trace(tfp, 99);
	tfp->open("simx.vcd");
	contextp->commandArgs(argc, argv);
	while(sim_time < MAX_SIM_TIME){
		sim_time++;
		contextp->timeInc(1);
		int a = rand() & 1;
		int b = rand() & 1;
		top->a = a;
		top->b = b;
		top->eval();
		tfp->dump(contextp->time());
		printf("a = %d, b = %d, f = %d\n", a, b, top->f);
		assert(top->f == a ^ b);
	}
	tfp->close();
	return 0;

}

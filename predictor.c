//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>
//
// TODO:Student Information
//
const char *studentName = "Raghav K Subramanian";
const char *studentID   = "A53282443";
const char *email       = "rksubram@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

int cmph=0;
int cmpl=0;
int end=0;
//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t global_history, local_history, global_mask, pc_mask, local_mask, global_prediction, local_prediction, choice_prediction;
uint32_t *global_history_table,*local_history_table,*local_prediction_table,*global_prediction_table, *choice_prediction_table;

int8_t perceptron_table[10000][10000];
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

uint32_t return_mask(uint32_t bits){
 //return the 2^(size)-1 as the number of bits in the BHT
 //For Example if we have a size of 5, we will have a total maximum BHT value of 11111, which corresponds to ba BHT size of 0-32
 //We will be masking 5 bits i.e taking 5 bits from the PC in this case
 uint32_t mask=(pow(2,bits)-1);
 return mask;
}

void assign_register( int size, uint32_t* table, uint32_t initial_value){
	for(int i=0;i<size;i++)
	{
		table[i]=initial_value; //initialize prediction to 0 for all bits as initial state required is weakly taken
	}
}
uint32_t mask_input(uint32_t mask_val, uint32_t input){
	return (mask_val & input);
}

void update_if_non_saturate(uint8_t outcome, uint32_t* table, uint32_t history){
	if(outcome==NOTTAKEN && table[history]!=0)
	{
		table[history]--;
	}
	else if(outcome==TAKEN && table[history]!=3)
	{
		table[history]++;
	}
//	return table[history];
}
// Initialize the predictor
//
void init_predictor()
{
	uint32_t BHT_size, GHT_size;
	uint32_t PT_size;
	//
	//TODO: Initialize Branch Predictor Data Structures
	//
	switch(bpType){
		case STATIC:
		case GSHARE:
			//G-Share
			global_history=0;
			global_mask=return_mask(ghistoryBits);
			BHT_size=global_mask+1;
			global_history_table= (uint32_t*) malloc((BHT_size)*(sizeof(uint32_t)));
			assign_register(BHT_size,global_history_table,1);
			return;
		case TOURNAMENT: 
			//Tournament
			//local history table
			pc_mask=return_mask(pcIndexBits);
			BHT_size=pc_mask+1;
			local_history_table= (uint32_t*) malloc((BHT_size)*(sizeof(uint32_t)));
			assign_register(BHT_size,local_history_table,0);
			//Local prediction table
			local_mask=return_mask(lhistoryBits);
			BHT_size=local_mask+1;
			local_prediction_table= (uint32_t*) malloc((BHT_size)*(sizeof(uint32_t)));
			assign_register(BHT_size,local_prediction_table,1);
			//Global prediction table
			global_mask=return_mask(ghistoryBits);
			GHT_size=global_mask+1;
			global_prediction_table=(uint32_t*) malloc((GHT_size)*(sizeof(uint32_t)));
			assign_register(GHT_size,global_prediction_table,1);
			//Choice prediction table
			global_mask=return_mask(ghistoryBits);
			GHT_size=global_mask+1;
			choice_prediction_table=(uint32_t*) malloc((GHT_size)*(sizeof(uint32_t)));
			assign_register(GHT_size,choice_prediction_table,2);
			return;
		case CUSTOM:
			ghistoryBits=10;
			lhistoryBits=10;
			pcIndexBits=10;
			pc_mask=return_mask(pcIndexBits);
			BHT_size=pc_mask+1;
			local_history_table= (uint32_t*) malloc((BHT_size)*(sizeof(uint32_t)));
			assign_register(BHT_size,local_history_table,0);
			//Local prediction table
			local_mask=return_mask(lhistoryBits);
			BHT_size=local_mask+1;
			local_prediction_table= (uint32_t*) malloc((BHT_size)*(sizeof(uint32_t)));
			assign_register(BHT_size,local_prediction_table,1);
			//Global prediction table
			global_mask=return_mask(ghistoryBits);
			GHT_size=global_mask+1;
			global_prediction_table=(uint32_t*) malloc((GHT_size)*(sizeof(uint32_t)));
			assign_register(GHT_size,global_prediction_table,1);
			//Choice prediction table
			global_mask=return_mask(ghistoryBits);
			GHT_size=global_mask+1;
			choice_prediction_table=(uint32_t*) malloc((GHT_size)*(sizeof(uint32_t)));
			assign_register(GHT_size,choice_prediction_table,2);
		default:
			break;
			//
	}
}
// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
{
	uint32_t gval,pcval,prediction,index;
	uint32_t perceptron_mask, GHR_val;
	int y;
	//
	//TODO: Implement prediction scheme
	//
	// Make a prediction based on the bpType
	switch (bpType) {
		case STATIC:
			return TAKEN;
		case GSHARE:
			gval=mask_input(global_mask, global_history);
			pcval=mask_input(global_mask, pc);
			index = gval ^ pcval;
			prediction= global_history_table[index];
			if(prediction>=2)
			{
				return TAKEN;
			}
			else
			{
				return NOTTAKEN;
			}
		case TOURNAMENT:
			gval=mask_input(global_mask,global_history);
			choice_prediction=choice_prediction_table[gval];
			pcval= mask_input(pc_mask,pc);
			local_history= mask_input(local_mask,local_history_table[pcval]);
			local_prediction=local_prediction_table[local_history];		 
			global_prediction=global_prediction_table[gval];
			prediction=(choice_prediction>=2) ? global_prediction:local_prediction;
			if(prediction>=2)
				return TAKEN;
			else
				return NOTTAKEN;
		case CUSTOM:
			gval=mask_input(global_mask,global_history);
			choice_prediction=choice_prediction_table[gval];
			pcval= mask_input(pc_mask,pc);
			local_history= mask_input(local_mask,local_history_table[pcval]);
			local_prediction=local_prediction_table[local_history];
			global_prediction=global_prediction_table[gval];
			prediction=(choice_prediction>=2) ? global_prediction:local_prediction;
			if(prediction>=2)
				return TAKEN;
			else
				return NOTTAKEN;
	}

	// If there is not a compatable bpType then return NOTTAKEN
	return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
	//
	//TODO: Implement Predictor training
	//
	uint32_t gval,pcval,prediction,index;
	uint32_t perceptron_mask,GHR_val,t,threshold;
	int PT_size,y;
	switch(bpType) {
		case GSHARE:
			gval=mask_input(global_mask, global_history);
			pcval=mask_input(global_mask, pc);
			index = gval ^ pcval;
			//prediction= global_history_table[index];
			update_if_non_saturate(outcome, global_history_table, index);
			global_history = outcome | (global_history<<1);
			return;
		case TOURNAMENT:
			gval= mask_input(global_mask, global_history);
			choice_prediction=choice_prediction_table[gval];
			pcval= mask_input(pc_mask, pc);
			local_history= mask_input(local_mask,local_history_table[pcval]);
			local_prediction=local_prediction_table[local_history];
			global_prediction=global_prediction_table[gval];	                			
			local_prediction=(local_prediction>=2)?TAKEN:NOTTAKEN;
			global_prediction=(global_prediction>=2)?TAKEN:NOTTAKEN;

			if(global_prediction==outcome && local_prediction!=outcome && choice_prediction_table[gval]!=3)
			{
				choice_prediction_table[gval]++;
			}
			else if(global_prediction!=outcome && local_prediction==outcome && choice_prediction_table[gval]!=0)
			{
				choice_prediction_table[gval]--;
			}
			update_if_non_saturate(outcome, local_prediction_table, local_history);
			update_if_non_saturate(outcome, global_prediction_table, gval);
			global_history= (outcome | (global_history<<1) ) & global_mask;
			local_history_table[pcval]= (outcome| (local_history_table[pcval] << 1)) & local_mask;
			return;
		case CUSTOM:
			gval= mask_input(global_mask, global_history);
			choice_prediction=choice_prediction_table[gval];
			pcval= mask_input(pc_mask, pc);
			local_history= mask_input(local_mask,local_history_table[pcval]);
			local_prediction=local_prediction_table[local_history];
			global_prediction=global_prediction_table[gval];
			local_prediction=(local_prediction>=2)?TAKEN:NOTTAKEN;
			global_prediction=(global_prediction>=2)?TAKEN:NOTTAKEN;

			if(global_prediction==outcome && local_prediction!=outcome && choice_prediction_table[gval]!=3)
			{
				choice_prediction_table[gval]++;
			}
			else if(global_prediction!=outcome && local_prediction==outcome && choice_prediction_table[gval]!=0)
			{
				choice_prediction_table[gval]--;
			}
			update_if_non_saturate(outcome, local_prediction_table, local_history);
			update_if_non_saturate(outcome, global_prediction_table, gval);
			global_history= (outcome | (global_history<<1) ) & global_mask;
			local_history_table[pcval]= (outcome| (local_history_table[pcval] << 1)) & local_mask;
			return;
		default:
			break; 
	}
}

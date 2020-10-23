#include "ultrascale_plus.h"
#include <stdlib.h>


#undef DEBUG_WHICH_RANGE
#undef DEBUG_OFFSET_RANGE
#undef VERBOSE

#define MAX_BITS_IN_FRAME (WORDS_PER_FRAME*32)

int incr_frame_repeat(int logicalm, int pos1, int pos2)
{
  int res=0;
  
  if ((logical_memories[logicalm].bit_locations[pos1].bit_loc==
       logical_memories[logicalm].bit_locations[pos2].bit_loc)
      &&
      ((logical_memories[logicalm].bit_locations[pos1].frame)==
       (logical_memories[logicalm].bit_locations[pos2].frame+1))
      )
    res=1;
  else
    res=0;

#ifdef DEBUG_INCR_FRAME_REPEAT
    printf("incr_frame_repeat called with %d %d frames %x %x bits %d %d --> %d\n",pos1,pos2,
	 logical_memories[logicalm].bit_locations[pos1].frame,
	 logical_memories[logicalm].bit_locations[pos2].frame,
	 logical_memories[logicalm].bit_locations[pos1].bit_loc,
	   logical_memories[logicalm].bit_locations[pos2].bit_loc,
	   res);
#endif  

  return(res);

}

// the frame given falls into which range of the frame ranges?
int which_range(int logicalm,int frame)
{
  for (int i=0;i<logical_memories[logicalm].nframe_ranges;i++)
    if ((logical_memories[logicalm].frame_ranges[i].first_frame<=frame)
	&& ((frame-logical_memories[logicalm].frame_ranges[i].first_frame)<
	    logical_memories[logicalm].frame_ranges[i].len))
      {
#ifdef DEBUG_WHICH_RANGE
	fprintf(stderr,"Which Range %d with frame 0x%x gets %d\n",logicalm,frame,i);
	fflush(stderr);
#endif	
	return(i);
      }
  fprintf(stderr,"which_range called with frame=0x%x not found in frame ranges for memory %d\n",
	  frame,logicalm);
  exit(3);
}

// which bram is this within the frame?
int offset_range(int bitloc)
{
  for (int i=0;i<WE_BITS_PER_FRAME;i++)
    {
#ifdef DEBUG_OFFSET_RANGE
      fprintf(stderr,"\tnow checking %d\n",i);
      fprintf(stderr,"\t&bram_starts=0x%x\n",i,&bram_starts);
      fprintf(stderr,"\t&bitlocation=0x%x\n",i,&bitlocation);
      fprintf(stderr,"\t&bram_starts[%d]=0x%x\n",i,&(bram_starts[i]));
      fprintf(stderr,"\t&bram_starts[%d]=0x%x\n",i+1,&(bram_starts[i+1]));
      fprintf(stderr,"\t&bitlocation[%d]=0x%x\n",i,&(bitlocation[i]));
      fprintf(stderr,"\t\tbram_starts[%d]=%d\n",i,bram_starts[i]);
#endif	
      if ((bram_starts[i]<=bitloc) && (bram_starts[i+1]>bitloc))
#ifdef DEBUG_OFFSET_RANGE
	fprintf(stderr,"Offset range %d gives %d\n",bitloc,i);
#endif	
	return(i);
    }
  fprintf(stderr,"offset_range called with bitloc=%d not in frame bits\n",
	  bitloc);
  exit(4);
}

int new_frame(int frame, int num_frames_seen, int *frames_seen)
{
  for (int i=0;i<num_frames_seen;i++)
    if (frames_seen[i]==frame)
      return(0);
  return(1);
}

int main ()
{
#ifdef VERBOSE  
  printf("//Design has %d logical memories\n", NUM_LOGICAL);
#endif
  printf("#include \"bert_types.h\"\n");
  printf("#define NUM_LOGICAL %d\n",NUM_LOGICAL);

  // reprint names
  printf("const char *logical_names[] = {");
  for(int i=0;i<NUM_LOGICAL;i++) {
    if (i!=0) printf(",");
    printf("\"%s\"",logical_names[i]);
  }
  printf("};\n");


  for(int i=0;i<NUM_LOGICAL;i++) {
    printf("// Beginning of Logical Memory %d\n",i);
#ifdef VERBOSE    
      printf("//Logical memory %d has name %s with organization %d x %d\n",
	     i,logical_names[i],
	     logical_memories[i].words,
	     logical_memories[i].wordlen);
#endif
      int *we_bits=(int *)malloc(sizeof(int)*logical_memories[i].nframe_ranges);
      int wordlen=logical_memories[i].wordlen;
      int current_frame=0;
      int bits_current_frame=0;
      int repeat_distance=-1;
      int repeat_slots=-1;
      int segments=0; // start with something not a segment
      int last_repeat_start=0;
      int last_repeat_slots=0;
      int segment_repeats=1;
      int frames_in_repeat=-1; // to deal with fake current_frame=0 ...
      int unique_frames_in_repeat=0;
      int *unique_frames=(int *)malloc(sizeof(int)*logical_memories[i].nframe_ranges);
      int bits_in_frame[MAX_BITS_IN_FRAME];
      for (int j=0;j<logical_memories[i].words;j++)
	{
	  for (int k=0;k<logical_memories[i].wordlen;k++)
	    {
#ifdef VERY_VERBOSE	      
	      printf("//word=%d, bit=%d, frame=0x%x, bitloc=%d\n",
		     j,k,
		     logical_memories[i].bit_locations[j*wordlen+k].frame,
		     logical_memories[i].bit_locations[j*wordlen+k].bit_loc);
#endif		     
	      
	      if ((repeat_distance>0) && (j*wordlen+k-last_repeat_start < repeat_distance))
		{
		  // check if following repeat
		  if (incr_frame_repeat(i,j*wordlen+k,j*wordlen+k-repeat_distance)!=1)
		    {
		      fprintf(stderr,"Warning: mismatch in repeat %d at pos=%d word=%d bit=%d for repeat_distance=%d (original repeat 0x%x %d) (now 0x%x %d)\n",segment_repeats,
			      j*wordlen+k,j,k,repeat_distance,
			      logical_memories[i].bit_locations[j*wordlen+k-repeat_distance].frame,
			      logical_memories[i].bit_locations[j*wordlen+k-repeat_distance].bit_loc,
			      logical_memories[i].bit_locations[j*wordlen+k].frame,
			      logical_memories[i].bit_locations[j*wordlen+k].bit_loc);
		      printf("Warning: mismatch in repeat %d at pos=%d word=%d bit=%d for repeat_distance=%d (original repeat 0x%x %d) (now 0x%x %d)\n",segment_repeats,
			     j*wordlen+k,j,k,repeat_distance,
			     logical_memories[i].bit_locations[j*wordlen+k-repeat_distance].frame,
			      logical_memories[i].bit_locations[j*wordlen+k-repeat_distance].bit_loc,
			      logical_memories[i].bit_locations[j*wordlen+k].frame,
			      logical_memories[i].bit_locations[j*wordlen+k].bit_loc);
		    }
		}
	      
	      if (logical_memories[i].bit_locations[j*wordlen+k].frame==current_frame)
		{
		  bits_in_frame[bits_current_frame]=logical_memories[i].bit_locations[j*wordlen+k].bit_loc;
		  bits_current_frame++;
		  we_bits[which_range(i,current_frame)]|=
		    (1<<offset_range(logical_memories[i].bit_locations[j*wordlen+k].bit_loc));


		}
	      else // frame changed
		{
#ifdef VERBOSE		  
		  printf("//\t\tFrame %x had %d bits:",current_frame,bits_current_frame);
		  for (int b=0;b<bits_current_frame;b++)
		    printf("%d,",bits_in_frame[b]);
		  printf("\n");
#endif
		  
		  // only print (insert) on first frame of segment
		  if (segment_repeats==1)
		    {
		      // don't insert if we just hit the repeat
		      if (incr_frame_repeat(i,j*wordlen+k,last_repeat_start)==0)
			if (new_frame(logical_memories[i].bit_locations[j*wordlen+k].frame,
				      unique_frames_in_repeat,unique_frames)==1)
			{
			  unique_frames[unique_frames_in_repeat]=logical_memories[i].bit_locations[j*wordlen+k].frame;
			  unique_frames_in_repeat++;
#ifdef VERBOSE_UNIQUE_SEGMENTS
			  printf("//Adding new frame 0x%x at word=%d, bit=%d in segment %d\n",
				 logical_memories[i].bit_locations[j*wordlen+k].frame,
				 j,k,segments);
#endif			  
			}
		      
		      if (bits_current_frame>0)
			{
			  printf("#define MEM%dSEG%dBITS_IN_FRAME%d %d\n",i,segments,frames_in_repeat,
				 bits_current_frame);
			  printf("int mem%dseg%dframe%dbits[%d]={%d",i,segments,frames_in_repeat,
				 bits_current_frame,
				 bits_in_frame[0]);
			  for (int b=1;b<bits_current_frame;b++)
			    printf(",%d",bits_in_frame[b]);
			  printf("};\n");
			  printf("#define MEM%dSEG%dFRAME%d {MEM%dSEG%dBITS_IN_FRAME%d,0x%x,mem%dseg%dframe%dbits}\n",i,segments,frames_in_repeat,i,segments,frames_in_repeat,current_frame,i,segments,frames_in_repeat);
			}
		    }
		    
		  bits_in_frame[0]=logical_memories[i].bit_locations[j*wordlen+k].bit_loc;		  
		  bits_current_frame=1;
		  current_frame=logical_memories[i].bit_locations[j*wordlen+k].frame;
		  we_bits[which_range(i,current_frame)]|=
		    (1<<offset_range(logical_memories[i].bit_locations[j*wordlen+k].bit_loc));


		  if (incr_frame_repeat(i,j*wordlen+k,last_repeat_start)==1)
		    {
		      
		      segment_repeats++;
		      int this_repeat_distance=j*wordlen+k-last_repeat_start;
		      if (repeat_distance<0) // first repeat found
			{
			  repeat_distance=this_repeat_distance;
			  repeat_slots=j-last_repeat_slots;
			  frames_in_repeat++; // so that is actual number for following
			  printf("//MEM%dSEG%d repeat distance %d covering %d frames\n",
				 i,segments,repeat_distance,frames_in_repeat);
			  printf("#define MEM%dSEG%dFRAMES_IN_REPEAT %d\n",i,segments,
				 frames_in_repeat);
			  printf("#define MEM%dSEG%dBITS_IN_REPEAT %d\n",i,segments,
				 repeat_distance);
			  printf("#define MEM%dSEG%dSLOTS_IN_REPEAT %d\n",i,segments,
				 repeat_slots);
			  //   array of frames
			  printf("struct frame_bits mem%dseg%dframes[%d]={",i,segments,frames_in_repeat);
			  for (int f=0;f<frames_in_repeat;f++)
			    {
			      if (f>0) printf(",");
			      printf("MEM%dSEG%dFRAME%d",i,segments,f);
			    }
			  printf("};\n");
			}
		      else // not first repeat
			{
			  if (repeat_distance!=this_repeat_distance)
			    fprintf(stderr,"//WARNING: found different repeat distances %d vs first %d\n", 
				   this_repeat_distance, repeat_distance);
			}
		      last_repeat_start=j*wordlen+k;
		      frames_in_repeat=0;
		    } // found repeat
		  else
		    {
		      if ((repeat_distance>0)
			  && (j*wordlen+k-last_repeat_start >= repeat_distance)) // possible new segment
			{
#ifdef VERBOSE			      
			  printf("//Segment %d had %d repeats\n",segments,segment_repeats);
#endif			      
			  // define repeats
			  printf("#define MEM%dSEG%dREPEATS %d\n",i,segments,segment_repeats);
			  printf("#define MEM%dSEG%dUNIQUE_FRAMES_IN_REPEAT %d\n",i,segments,unique_frames_in_repeat);
			  printf("int mem%dseg%dunique_frames[%d]= {",i,segments,unique_frames_in_repeat);
			  for(int u=0;u<unique_frames_in_repeat;u++)
			    {
			      if (u!=0) printf(",");
			      printf("0x%x",unique_frames[u]);
			    }
			  printf("};\n");
			  
			  // structure to put together repeats and frames in repeat
			  printf("#define MEM%dSEG%d {MEM%dSEG%dREPEATS,MEM%dSEG%dFRAMES_IN_REPEAT,MEM%dSEG%dBITS_IN_REPEAT,MEM%dSEG%dSLOTS_IN_REPEAT,MEM%dSEG%dUNIQUE_FRAMES_IN_REPEAT,mem%dseg%dframes,mem%dseg%dunique_frames}\n",i,segments,i,segments,i,segments,i,segments,i,segments,i,segments,i,segments,i,segments);
			  
			  segments++;
			  segment_repeats=1;
			  last_repeat_start=j*wordlen+k;
			  last_repeat_slots=j;
			  frames_in_repeat=0; 
			  repeat_distance=-1; // restart
			  repeat_slots=-1;
			  unique_frames_in_repeat=1;
			  unique_frames[0]=logical_memories[i].bit_locations[j*wordlen+k].frame;
			  bits_in_frame[0]=logical_memories[i].bit_locations[j*wordlen+k].bit_loc;
			  bits_current_frame=1;
			  current_frame=logical_memories[i].bit_locations[j*wordlen+k].frame;

#ifdef VERBOSE_UNIQUE_SEGMENTS
			  printf("//Adding new frame 0x%x bit_loc=%d at word=%d, bit=%d in segment %d\n",
				 logical_memories[i].bit_locations[j*wordlen+k].frame,
				 logical_memories[i].bit_locations[j*wordlen+k].bit_loc,
				 j,k,segments);
#endif			  
			  
			}
		      else
	
			frames_in_repeat++;
		    }
		}
#ifdef VERY_VERBOSE	      
	      printf("\t%s[%d][%d] comes from frame=%x bit=%d\n",
		     logical_names[i],j,k,
		     logical_memories[i].bit_locations[j*wordlen+k].frame,
		     logical_memories[i].bit_locations[j*wordlen+k].bit_loc);
#endif
	      
	    }
	  
	}
      // need this for last frame?
      // TODO: worry about this possibly being first frame...then will need to do something
#ifdef VERBOSE      
      printf("\t\tFrame %x had %d bits:",current_frame,bits_current_frame);
      for (int b=0;b<bits_current_frame;b++)
	printf("%d,",bits_in_frame[b]);
      printf("\n");
#endif      
#ifdef VERBOSE      
      printf("//Segment %d had %d repeats\n",segments,segment_repeats);
#endif      
      // define repeats
      printf("#define MEM%dSEG%dREPEATS %d\n",i,segments,segment_repeats);
      printf("#define MEM%dSEG%dUNIQUE_FRAMES_IN_REPEAT %d\n",i,segments,unique_frames_in_repeat);
      printf("int mem%dseg%dunique_frames[%d]= {",i,segments,unique_frames_in_repeat);
      for(int u=0;u<unique_frames_in_repeat;u++)
	{
	  if (u!=0) printf(",");
	  printf("0x%x",unique_frames[u]);
	}
      printf("};\n");
			  
      // structure to put together repeats and frames in repeat
      printf("#define MEM%dSEG%d {MEM%dSEG%dREPEATS,MEM%dSEG%dFRAMES_IN_REPEAT,MEM%dSEG%dBITS_IN_REPEAT,MEM%dSEG%dSLOTS_IN_REPEAT,MEM%dSEG%dUNIQUE_FRAMES_IN_REPEAT,mem%dseg%dframes,mem%dseg%dunique_frames}\n",i,segments,i,segments,i,segments,i,segments,i,segments,i,segments,i,segments,i,segments);
      // define segments
      printf("#define MEM%dSEGMENTS %d\n",i,segments+1);
      // array for segments
      printf("struct segment_repeats mem%drepeats[%d]={",i,segments+1);
      for (int s=0;s<segments+1;s++)
	{
	  if (s!=0) printf(",");
	  printf("MEM%dSEG%d",i,s);
	}
      printf("};\n");
      // structure for....
      //printf("struct memory_segments mem%dcomposition={MEM%dSEGMENTS,mem%dsegments};\n",i,i,i);

      // write out write enables for memory
      for (int w=0;w<logical_memories[i].nframe_ranges;w++)
	{
	  printf("#define MEM%dFR%dWEBITS (0x%x)\n",
		 i,w,we_bits[w]);
	}
      printf("// End of Logical Memory %d\n",i);
  }

  // rewrite  frame ranges -- now with write enables
  for(int i=0;i<NUM_LOGICAL;i++) {
    printf("struct frame_range mem%d_frame_ranges[%d] = {",
	   i,logical_memories[i].nframe_ranges);
    
	     
      for (int j=0;j<logical_memories[i].nframe_ranges;j++)
	{
	  printf("{0x%x,%d,MEM%dFR%dWEBITS}",
		 logical_memories[i].frame_ranges[j].first_frame,
		 logical_memories[i].frame_ranges[j].len,
		 i,j
		 );
	  if (j!=(logical_memories[i].nframe_ranges-1))
	    printf(",");
	}
      printf(" };\n");
  }

  // array for all logical memories
  printf("struct logical_memory logical_memories[NUM_LOGICAL] ={\n");
  for(int i=0;i<NUM_LOGICAL;i++) {
    
    printf("\t{%d,%d,%d,MEM%dSEGMENTS,mem%d_frame_ranges,mem%drepeats}\n",
	   logical_memories[i].nframe_ranges,
	   logical_memories[i].wordlen,
	   logical_memories[i].words,
	   i,i,i);
    if (i!=(NUM_LOGICAL-1))
      printf(",");

  }
  printf("};\n");

}

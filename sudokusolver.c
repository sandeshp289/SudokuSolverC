////////////////////////////////////////////////////////////////////////
// File       : sudokusolver.c
// Language   : C/C++
// Description: Sudoku Solver
// Author     : Sandesh Prabhakar
// Created    : September 7, 2021
// Modified   : August 15, 2026
// Version    : 1.0
//
// Copyright (c) 2026 Sandesh Prabhakar
// All rights reserved
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

////////////////////////////////////////////////////////////////////////
// circuit class
////////////////////////////////////////////////////////////////////////

class circuit
{
    int temp;                    // temporary variable to store minterms and dont cares while reading in
    int **MAT;
    int ****table;
    int ****table_copy;
    int *global_change_flag;

    // you may wish to add more private variables here

public:
    circuit(char *);   // constructor

    // you may wish to add more function declarations (or public variables) here
        int print_sudoku (int**** table);
        int print_sudoku_unsolved_count (int**** table);
        int print_sudoku_unsolved_choices (int**** table);
        int parse_and_optimize (int**** table);
        int row_column_block_compare_and_optimize (int**** table);
        int row_column_block_naked_subset_and_optimize (int**** table);
        int sudoku_solver_basic(int****table, int* global_change_flag);
        int calculate_sudoku_unsolved_count (int**** table);
        int verify_sudoku (int**** table);
        int copy_table(int**** table, int**** table_copy);
        int sudoku_solver_advanced(int**** table, int**** table_copy,int* global_change_flag);

};

////////////////////////////////////////////////////////////////////////
// constructor: reads in the *.cubes file and builds basic data structure
// 		for the cube list 
////////////////////////////////////////////////////////////////////////
circuit::circuit(char *cktName)
{
    FILE *inFile;
    FILE *outFile;
    char fName[40];
    char foName[40];
    int i,j,k,r,cl,bx,cx,rx,b;
    char c;
    strcpy(fName, cktName);
    strcat(fName, ".su");

    inFile = fopen(fName, "r");
    if (inFile == NULL)
    {
	fprintf(stderr, "Can't open .sop file\n");
	exit(-1);
    }

        MAT = new int * [10];
        table = new int *** [10];
        table_copy = new int *** [10];

        for(int i=0;i<10;i++)
            {
            table[i]=new int ** [10];
            table_copy[i]=new int ** [10];
               for(int j=0; j<10;j++)
                  {
                   table[i][j]=new int * [10];
                   table_copy[i][j]=new int * [10];
                    for(int k=0;k<10;k++)
                      {
                      table[i][j][k]=new int [10];
                      table_copy[i][j][k]=new int [10];
                      }
                  }
            }

          //Initializing the scheduling list
	 for(int i=0; i<10;i++)
	 {
            for (int j=0; j<10; j++)
	    {
               for (int k=0; k<10; k++)
                    {
                    for(int l=0; l<10;l++)
                    {
		    table[i][j][k][l] = 0;
		    table_copy[i][j][k][l] = 0;
                    }
                    }
            }
        }

	for (i=0; i<10; i++)
	{
                MAT[i] = new int[10];
		for (j=0; j<10; j++)
		{
		    MAT[i][j] = 0;
		}
	}

rx=0;
cx=0;
bx=1;
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
        	fscanf(inFile, "%d", &temp);
                if(temp==0)
                {
                table[bx][r][cl][0]=9;
                   for(i=1;i<10;i++)
                     {
                     table[bx][r][cl][i]=i;
                     }
                }
                else
                {
                     table[bx][r][cl][0]=1;
                     table[bx][r][cl][1]=temp;
                }
             
              if(cx==3)
                {
                bx++;
                cx=0;
                }

         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }

	// skip till end of line
	while ((c = getc(inFile)) != '\n' && c != EOF)
	    ;

    fclose(inFile);


    printf("Successfully read in Sudoku:\n");
    printf("\tPrinting input Sudoku\n");
    print_sudoku(table);

global_change_flag = new int[1];
global_change_flag[0]=1;
int unsolved_count=0;
int error_flag=0;

error_flag=verify_sudoku(table);
if(error_flag==1)
printf("ERROR IN INPUT SUDOKU\n");

if(error_flag==0)
sudoku_solver_basic(table,global_change_flag);
error_flag=verify_sudoku(table);

unsolved_count=calculate_sudoku_unsolved_count(table);

copy_table(table,table_copy);

if(unsolved_count==81 && error_flag==0)
    {
    printf("Successfully processed Sudoku:\n");
    printf("\tPrinting output Sudoku\n");
    print_sudoku(table);
    }

if(unsolved_count>81 && error_flag==0)
   {
    sudoku_solver_advanced(table,table_copy,global_change_flag);

    unsolved_count=calculate_sudoku_unsolved_count(table_copy);

    if(unsolved_count==81)
      {
       printf("Successfully processed Scary Sudoku:\n");
       printf("\tPrinting output Sudoku\n");
       print_sudoku(table_copy);
      }
     else
      {
       printf("\tPrinting output Sudoku\n");
       print_sudoku(table_copy);
       printf("\tPrinting output Sudoku based on unsolved count\n");
       print_sudoku_unsolved_count(table_copy);
       printf("\tPrinting output Sudoku based on unsolved choices\n");
       print_sudoku_unsolved_choices(table_copy);
      }
   }

}

////////////////////////////////////////////////////////////////////////
// other member functions here
////////////////////////////////////////////////////////////////////////

int circuit::sudoku_solver_advanced(int**** table, int**** table_copy,int* global_change_flag)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
int unsolved_count=0;
int error_flag=0;

    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                if(table[bx][r][cl][0]==2)
                {
                 for(int z=1;z<10;z++)
                 {
                  copy_table(table,table_copy);
                  if(table[bx][r][cl][z]>0) 
                  {
                  table_copy[bx][r][cl][0]=1;
                    for(int ix=1;ix<10;ix++)
                     {
                       if(ix==table[bx][r][cl][z])
                       {
                       table_copy[bx][r][cl][z]=table[bx][r][cl][z];
                       }
                       else
                       {
                       table_copy[bx][r][cl][z]=0;
                       }
                     }
                  }
                  
                   global_change_flag[0]=1;
                   unsolved_count=0;
                   error_flag=0;
                   sudoku_solver_basic(table_copy,global_change_flag);
                   unsolved_count=calculate_sudoku_unsolved_count(table_copy);
                   error_flag=verify_sudoku(table_copy);

                  if(unsolved_count==81 && error_flag==0)
                  break;
                 }
                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         if(unsolved_count==81 && error_flag==0)
         break;
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
     //printf("\n");
    if(unsolved_count==81 && error_flag==0)
    break;
    }

}


int circuit::copy_table(int**** table, int**** table_copy)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                for(int z=0;z<10;z++)
                 {
                  table_copy[bx][r][cl][z]=table[bx][r][cl][z]; 
                 }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }
}



int circuit::sudoku_solver_basic(int****table, int* global_change_flag)
 {

int temp_flag;
int loop_count=0;

   while(global_change_flag[0])
   {

       global_change_flag[0]=parse_and_optimize(table);

       temp_flag=row_column_block_compare_and_optimize(table);
       if(temp_flag==1 && global_change_flag[0]==0)
        {
         global_change_flag[0]=1;
        }

       temp_flag=row_column_block_naked_subset_and_optimize(table);
       if(temp_flag==1 && global_change_flag[0]==0)
        {
         global_change_flag[0]=1;
        }
       loop_count++;
   }

}



int circuit::verify_sudoku (int**** table)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
int temp_MAT[10][10];
int temp_MAT_BL[10][10];
int bx_index[10];
int error_flag=0;
int i,j;

     for(i=0;i<10;i++)
      {
       bx_index[i]=0;
       for(j=0;j<10;j++)
         {
          temp_MAT[i][j]=0;
          temp_MAT_BL[i][j]=0;
         }
      }

    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                if(table[bx][r][cl][0]==1)
                {
                 for(int z=1;z<10;z++)
                 {
                  if(table[bx][r][cl][z]>0) 
                  {
                  temp_MAT[r][cl]=table[bx][r][cl][z];
                  bx_index[bx]=bx_index[bx]+1;
                  temp_MAT_BL[bx][bx_index[bx]]=table[bx][r][cl][z];
                  }
                 }
                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }


         for(r=1; r<10; r++)
          {
           for(int cr=1;cr<9;cr++)
              {
               for(int crx=cr+1;crx<10;crx++)
                  {
                   if(temp_MAT[r][cr]==temp_MAT[r][crx])
                    {
                     if(temp_MAT[r][cr]!=0)
                     {
                     error_flag=1;
                     break;
                     }
                    }
                  }
               if(error_flag==1)
               break;
              }
               if(error_flag==1)
               break;
         }

    if(error_flag==0)
     {
        for(cl=1; cl<10; cl++)
          {
           for(int cr=1;cr<9;cr++)
              {
               for(int crx=cr+1;crx<10;crx++)
                  {
                   if(temp_MAT[cr][cl]==temp_MAT[crx][cl])
                    {
                     if(temp_MAT[cr][cl]!=0)
                     {
                     error_flag=1;
                     break;
                     }
                    }
                  }
               if(error_flag==1)
               break;
              }
         if(error_flag==1)
         break;
         }
     }


    if(error_flag==0)
     {
        for(bx=1; bx<10; bx++)
          {
           for(int cr=1;cr<9;cr++)
              {
                   if(temp_MAT_BL[bx][cr]==temp_MAT_BL[bx][cr+1])
                    {
                     if(temp_MAT_BL[bx][cr]!=0)
                     {
                     error_flag=1;
                     break;
                     }
                    }
               if(error_flag==1)
               break;
              }
         if(error_flag==1)
         break;
         }
     }

return error_flag;

}

int circuit::print_sudoku (int**** table)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                if(table[bx][r][cl][0]>1)
                {
                printf("0 ");
                }
                else
                {
                 for(int z=1;z<10;z++)
                 {
                  if(table[bx][r][cl][z]>0) 
                  printf("%d ", table[bx][r][cl][z]);
                 }
                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
     printf("\n");
    }
}


int circuit::print_sudoku_unsolved_count (int**** table)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;

    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;

              if(table[bx][r][cl][0]>0)
                {
                printf("%d ",table[bx][r][cl][0]);
                }
     
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
     printf("\n");
    }

}

int circuit::print_sudoku_unsolved_choices (int**** table)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
char chk = '\t';
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                if(table[bx][r][cl][0]>1)
                {
                 for(int z=1;z<10;z++)
                  {
                   if(table[bx][r][cl][z]>0)
                   printf("%d",table[bx][r][cl][z]);
                  }
                  printf("%c",chk);
                }
                else
                {
                 for(int z=1;z<10;z++)
                 {
                  if(table[bx][r][cl][z]>0) 
                  printf("%-7d ", table[bx][r][cl][z]);
                 }
                }

              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
     printf("\n");
    }
}

int circuit::calculate_sudoku_unsolved_count (int**** table)
{
int rx=0;
int cx=0;
int bx=1;
int r,cl,z;
int count=0;
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
              if(table[bx][r][cl][0]>0)
                {
                count=count+table[bx][r][cl][0];
                }
     
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }
return count;
}


int circuit::parse_and_optimize (int**** table)
{
int loop_count=0;
int change_flag=1;
int global_change_flag=0;
while(change_flag)
  {

int rx=0;
int cx=0;
int bx=1;
int tempx,i,j,r,cl,z,b;
   change_flag=0; 
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
                cx++;
                if(table[bx][r][cl][0]==1)
                {
                 for(z=1;z<10;z++)
                 {
                  if(table[bx][r][cl][z]>0) 
                  tempx=table[bx][r][cl][z];
                 }

                  for(i=1;i<10;i++)
                   {
                      for(b=1;b<10;b++)
                      {
                        if(table[b][r][i][0]>1)
                         {
                          if(table[b][r][i][tempx]!=0)
                           {
                            table[b][r][i][tempx]=0;
                            table[b][r][i][0]=table[b][r][i][0]-1;
                            change_flag=1;
                            global_change_flag=1;
                           }
                         }
                      }
                   }

                  for(i=1;i<10;i++)
                   {
                      for(b=1;b<10;b++)
                      {
                        if(table[b][i][cl][0]>1)
                         {
                          if(table[b][i][cl][tempx]!=0)
                           {
                            table[b][i][cl][tempx]=0;
                            table[b][i][cl][0]=table[b][i][cl][0]-1;
                            change_flag=1;
                            global_change_flag=1;
                           }
                         }
                      }
                   }

                  
                  for(i=1;i<10;i++)
                   {
                   for(j=1;j<10;j++)
                    {
                    if(table[bx][i][j][0]>1)
                     {
                       if(table[bx][i][j][tempx]!=0)
                        {
                        table[bx][i][j][tempx]=0;
                        table[bx][i][j][0]=table[bx][i][j][0]-1;
                        change_flag=1;
                        global_change_flag=1;
                        }
                     }
                    }
                   }

                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }
loop_count++;
}


return global_change_flag;
}



int circuit::row_column_block_compare_and_optimize (int**** table)
{
int loop_count=0;
int change_flag=1;
int global_change_flag=0;

while(change_flag)
  {

int rx=0;
int cx=0;
int bx=1;
int tempx,tempx2,r,cl,z,i,j,jj,m,b;
int clear_flag=1;
   change_flag=0; 
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
            cx++;
            if(table[bx][r][cl][0]>1)
             {
                 for( z=1;z<10;z++)
                 {

                     if(table[bx][r][cl][z]>0) 
                      {
                      clear_flag=0;

                       tempx=table[bx][r][cl][z];
                                 for(i=1;i<10;i++)
                                   {
                                    if(i!=cl)
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                             if(table[b][r][i][0]>0)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][r][i][j];
                                                       if(tempx2==tempx)
                                                         {
                                                         clear_flag=1;
                                                         break;
                                                         }
                                                   }
                                               }
                                           if(clear_flag==1)
                                           break;
                                         }
                                      }
                                   if(clear_flag==1)
                                   break;
                                   }

                               if(clear_flag==1)
                                 {
                                 clear_flag=0;
                                 for(i=1;i<10;i++)
                                   {
                                    if(i!=r)
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                             if(table[b][i][cl][0]>0)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][i][cl][j];
                                                       if(tempx2==tempx)
                                                         {
                                                         clear_flag=1;
                                                         break;
                                                         }
                                                   }
                                               }
                                           if(clear_flag==1)
                                           break;
                                         }
                                      }
                                   if(clear_flag==1)
                                   break;
                                   }
                                  }


                               if(clear_flag==1)
                                 {
                                 clear_flag=0;
                                   for(i=1;i<10;i++)
                                     {
                                       for(j=1;j<10;j++)
                                            {
                                             if((i!=r) || (j!=cl))
                                               {
                                                if(table[bx][i][j][0]>0)
                                                   {
                                                    for(jj=1;jj<10;jj++)
                                                        {
                                                          tempx2=table[bx][i][j][jj];
                                                          if(tempx2==tempx)
                                                            {
                                                             clear_flag=1;
                                                             break;
                                                            }
                                                        }
                                                    }
                                                }
                                              if(clear_flag==1)
                                              break;
                                             }
                                      if(clear_flag==1)
                                      break;
                                     }
                                  }

                      }
                        if(clear_flag==0)
                         {
                           for(m=1;m<10;m++)
                             {
                              if(m==tempx)
                               {
                                table[bx][r][cl][m]=m;
                                table[bx][r][cl][0]=1;
                               }
                               else
                               {
                               table[bx][r][cl][m]=0;
                               }
                             }
                         change_flag=1;
                         global_change_flag=1;
                         clear_flag=1;
                         break;
                         }
                }

 
                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }
loop_count++;
}

return global_change_flag;
}


int circuit::row_column_block_naked_subset_and_optimize (int**** table)
{
int loop_count=0;
int change_flag=1;
int global_change_flag=0;

while(change_flag)
  {

int rx=0;
int cx=0;
int bx=1;
int tempx,tempx2,r,cl,z,i,j,jj,m,b;
int clear_flag=1;
int crb,crb1,crb2;
int *store;
store=new int [3];
store[0]=0;
store[1]=0;
store[2]=0;

   change_flag=0; 
    for (r=1; r<10; r++)
    {
      rx++;
      for(cl=1;cl<10;cl++)
        {
            cx++;
            if(table[bx][r][cl][0]==2)
             {
                 store[0]=0;
                 store[1]=0;
                 store[2]=0; 
                    for( z=1;z<10;z++)
                    {
                     if(table[bx][r][cl][z]>0) 
                      {
                      store[0]++;
                      int xyz=store[0];
                      store[xyz]=table[bx][r][cl][z];
                      }
                     }

//ROW fixed
                      clear_flag=0;

                                 for(i=1;i<10;i++)
                                   {
                                    if(i>cl)
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                          clear_flag=0;
                                             if(table[b][r][i][0]==2)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][r][i][j];
                                                       if(tempx2==store[1])
                                                         {
                                                         clear_flag++;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         clear_flag++;
                                                         }
                                                    crb=i;
                                                   }
                                               }
                                           if(clear_flag==2)
                                           break;
                                         }
                                      }
                                   if(clear_flag==2)
                                   break;
                                   }

                               if(clear_flag==2)
                                 {
                                 clear_flag=0;
                                 for(i=1;i<10;i++)
                                   {
                                    if((i!=cl)&&(i!=crb))
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                             if(table[b][r][i][0]>0)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][r][i][j];
                                                       if(tempx2==store[1])
                                                         {
                                                         table[b][r][i][j]=0;
                                                         table[b][r][i][0]=table[b][r][i][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         table[b][r][i][j]=0;
                                                         table[b][r][i][0]=table[b][r][i][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                   }
                                               }
                                         }
                                      }
                                   }
                                  }


////Column fixed
                      clear_flag=0;

                                 for(i=1;i<10;i++)
                                   {
                                    if(i>r)
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                            clear_flag=0;
                                             if(table[b][i][cl][0]==2)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][i][cl][j];
                                                       if(tempx2==store[1])
                                                         {
                                                         clear_flag++;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         clear_flag++;
                                                         }
                                                    crb=i;
                                                   }
                                               }
                                           if(clear_flag==2)
                                           break;
                                         }
                                      }
                                   if(clear_flag==2)
                                   break;
                                   }

                               if(clear_flag==2)
                                 {
                                 clear_flag=0;
                                 for(i=1;i<10;i++)
                                   {
                                    if((i!=r)&&(i!=crb))
                                      {
                                        for(b=1;b<10;b++)
                                         {
                                             if(table[b][i][cl][0]>0)
                                               {
                                                 for(j=1;j<10;j++)
                                                   {
                                                   tempx2 = table[b][i][cl][j];
                                                       if(tempx2==store[1])
                                                         {
                                                         table[b][i][cl][j]=0;
                                                         table[b][i][cl][0]=table[b][i][cl][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         table[b][i][cl][j]=0;
                                                         table[b][i][cl][0]=table[b][i][cl][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                   }
                                               }
                                         }
                                      }
                                   }
                                  }

////Block fixed
                                 clear_flag=0;
                                   for(i=1;i<10;i++)
                                     {
                                       for(j=1;j<10;j++)
                                            {
                                             clear_flag=0;
                                             if((i!=r) || (j!=cl))
                                               {
                                                if(table[bx][i][j][0]==2)
                                                   {
                                                    for(jj=1;jj<10;jj++)
                                                     {
                                                      tempx2 = table[bx][i][j][jj];
                                                       if(tempx2==store[1])
                                                         {
                                                         clear_flag++;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         clear_flag++;
                                                         }
                                                    crb1=i;
                                                    crb2=j;
                                                     }
                                                   }
                                                 }
                                            if(clear_flag==2)
                                            break;
                                               }
                                        if(clear_flag==2)
                                        break;
                                        }


                              if(clear_flag==2)
                                 {
                                 clear_flag=0;
                                 for(i=1;i<10;i++)
                                   {
                                    for(j=1;j<10;j++)
                                      {
                                         if(((i!=r)||(j!=cl)))
                                          {

                                           if(((i!=crb1)||(j!=crb2)))
                                            {
                                             if(table[bx][i][j][0]>0)
                                               {
                                                 for(jj=1;jj<10;jj++)
                                                   {
                                                   tempx2 = table[bx][i][j][jj];
                                                       if(tempx2==store[1])
                                                         {
                                                         table[bx][i][j][jj]=0;
                                                         table[bx][i][j][0]=table[bx][i][j][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                       if(tempx2==store[2])
                                                         {
                                                         table[bx][i][j][jj]=0;
                                                         table[bx][i][j][0]=table[bx][i][j][0]-1;
                                                         change_flag=1;
                                                         global_change_flag=1;
                                                         }
                                                   }
                                               }
                                             }
                                          }
                                      }
                                   }
                                  }

                }

             
              if(cx==3)
                {
                bx++;
                cx=0;
                }
         }

     if(rx==3)
       {
       rx=0;
       }
     else
       {
       bx=bx-3;
       }
    }
loop_count++;
}

return global_change_flag;
}

//
////////////////////////////////////////////////////////////////////////
// main starts here
////////////////////////////////////////////////////////////////////////
main(int argc, char *argv[])
{
    char cktName[20];
    circuit *ckt;
    int vecWidth, poNum;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        exit(-1);
    }

    strcpy(cktName, argv[1]);

    // read in sudoku and build data structure
    ckt = new circuit(cktName);

    // NOW ADD YOUR FUNCTION HERE
}

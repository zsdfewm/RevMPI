c   DESIRED TAPENADE ADJOINT USING THE AMPI LIBRARY
c   One proc just sends, one proc just receives

c Differentiation of head in reverse (adjoint) mode:
c  gradient     of useful results: v1 v2
c  with respect to varying inputs: v1 v2
c  RW status of diff variables: v1:incr v2:in-out
c  Plus diff mem management of: v1:in v2:in
      SUBROUTINE HEAD_B(v1, v1b, v2, v2b)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION v1, v2
      DOUBLE PRECISION v1b, v2b
      INTEGER world_rank, err_code
      DOUBLE PRECISION value
      DOUBLE PRECISION valueb

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         value = SQRT(v1)
         call FW_AMPI_Send(value, 1, AMPI_ADOUBLE_PRECISION,
     +        1, 0, AMPI_TO_RECV, MPI_COMM_WORLD, err_code)
         call BW_AMPI_Send(valueb, 1, AMPI_ADOUBLE_PRECISION,
     +        1, 0, AMPI_TO_RECV, MPI_COMM_WORLD, err_code)
         IF (v1.NE.0.0) v1b = v1b + valueb/(2.0*SQRT(v1))
      ELSE IF (world_rank.EQ.1) THEN
         call FW_AMPI_Recv(value, 1, AMPI_ADOUBLE_PRECISION,0, 0,
     +        AMPI_FROM_SEND,MPI_COMM_WORLD,MPI_STATUS_IGNORE,err_code)
         valueb = COS(value)*v2b
         call BW_AMPI_Recv(valueb, 1, AMPI_ADOUBLE_PRECISION,0, 0,
     +        AMPI_FROM_SEND,MPI_COMM_WORLD,MPI_STATUS_IGNORE,err_code)
        v2b = 0.0
      ENDIF
      END

      PROGRAM MAIN
      implicit none
      include 'ampi/ampif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xb,yb

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         xb=0.0
         x=3.5
         print *,"process",world_rank," sets val  [3.50000000==]",x
         call HEAD_B(x,xb,y,yb)
         print *,"process",world_rank," gets diff [-7.898936E-002==]",xb
      ELSE IF (world_rank.EQ.1) THEN
         yb=1.0
         y=0.0
         print *,"process",world_rank," sets diff [1.00000000==]",yb
         call HEAD_B(x,xb,y,yb)
         print *,"process",world_rank," gets val  [0.00000000==]",y
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

c   DESIRED TAPENADE ADJOINT USING THE AMPI LIBRARY
c Simple Irecv+Send+Wait || Irecv+Wait+Send

c Differentiation of head in reverse (adjoint) mode:
c  gradient     of useful results: x y
c  with respect to varying inputs: x y
c  RW status of diff variables: x:incr y:in-out
c  Plus diff mem management of: x:in y:in
      SUBROUTINE HEAD_B(x, xb, y, yb)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x, y
      DOUBLE PRECISION xb, yb
      INTEGER world_rank, err_code, req
      DOUBLE PRECISION local
      DOUBLE PRECISION localb

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         call FW_AMPI_Irecv(y, 1, AMPI_ADOUBLE_PRECISION,
     +         1, 0, AMPI_FROM_SEND,
     +        MPI_COMM_WORLD, req, err_code)
         x=x*2.0
         call FW_AMPI_Send(x, 1, AMPI_ADOUBLE_PRECISION,
     +         1, 0, AMPI_TO_IRECV_WAIT,
     +        MPI_COMM_WORLD, err_code)
         call FW_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         call ADTOOL_AMPI_Turn(x,xb)
         call ADTOOL_AMPI_Turn(y,yb)
         yb = 3.0*yb
         call BW_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         call BW_AMPI_Send(xb, 1, AMPI_ADOUBLE_PRECISION,
     +         1, 0, AMPI_TO_IRECV_WAIT,
     +        MPI_COMM_WORLD, err_code)
         xb = xb*2.0
         call BW_AMPI_Irecv(yb, 1, AMPI_ADOUBLE_PRECISION,
     +         1, 0, AMPI_FROM_SEND,
     +        MPI_COMM_WORLD, req, err_code)
      ELSE IF (world_rank.EQ.1) THEN
         call FW_AMPI_Irecv(local, 1, AMPI_ADOUBLE_PRECISION,
     +         0, 0, AMPI_FROM_SEND,
     +        MPI_COMM_WORLD, req, err_code)
         call FW_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         call pushreal8(local)
         local = SIN(local)
         call FW_AMPI_Send(local, 1, AMPI_ADOUBLE_PRECISION,
     +         0, 0, AMPI_TO_IRECV_WAIT,
     +        MPI_COMM_WORLD, err_code)
         call ADTOOL_AMPI_Turn(local,localb)
         call BW_AMPI_Send(localb, 1, AMPI_ADOUBLE_PRECISION,
     +         0, 0, AMPI_TO_IRECV_WAIT,
     +        MPI_COMM_WORLD, err_code)
         call popreal8(local)
         localb = COS(local)*localb
         call BW_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         call BW_AMPI_Irecv(localb, 1, AMPI_ADOUBLE_PRECISION,
     +         0, 0, AMPI_FROM_SEND,
     +        MPI_COMM_WORLD, req, err_code)
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
         yb=1.0
         xb=0.0
         x=2.5
         print *,"process",world_rank,
     +        " sends val  [2.500000000000==]",x
         call HEAD_B(x,xb,y,yb)
         print *,"process",world_rank,
     +        " recvs diff [1.701973112779==]",xb
      ELSE IF (world_rank.EQ.1) THEN
         call HEAD_B(x,xb,y,yb)
      ENDIF
      call AMPI_Finalize_NT(err_code)
      RETURN
      END

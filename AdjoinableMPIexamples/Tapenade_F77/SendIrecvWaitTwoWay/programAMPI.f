C Hand-turned into AMPI. Not differentiated.
c    Simple Irecv+Send+Wait || Irecv+Wait+Send

      SUBROUTINE head(x,y)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x,y
      INTEGER world_rank, err_code, req
      DOUBLE PRECISION local

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         call AMPI_IRecv(y, 1, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_FROM_SEND, MPI_COMM_WORLD, req, err_code)
         x=x*2
         call AMPI_Send(x, 1, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, err_code)
         call AMPI_Wait(req,MPI_STATUS_IGNORE, err_code)
         y=y*3
      ELSE IF (world_rank.EQ.1) THEN
         call AMPI_IRecv(local, 1, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_FROM_SEND, MPI_COMM_WORLD, req, err_code)
         call AMPI_Wait(req,MPI_STATUS_IGNORE, err_code)
         local = SIN(local)
         call AMPI_Send(local, 1, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION x,y

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         x=2.5
         print *,"process",world_rank," sends val  [ 2.500000000==]",x
         call head(x,y)
         print *,"process",world_rank," recvs val  [-2.876772823==]",y
         y=y+x
      ELSE IF (world_rank.EQ.1) THEN
         call head(x,y)
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

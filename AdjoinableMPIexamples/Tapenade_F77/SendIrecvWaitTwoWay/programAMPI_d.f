c   DESIRED TAPENADE TANGENT USING THE AMPI LIBRARY
c    Simple Irecv+Send+Wait || Irecv+Wait+Send

      SUBROUTINE head_d(x,xd,y,yd)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xd,yd
      INTEGER world_rank, err_code, req
      DOUBLE PRECISION local
      DOUBLE PRECISION locald

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         call TLS_AMPI_IRecv(y, yd, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_FROM_SEND, MPI_COMM_WORLD, req, err_code)
         xd=xd*2
         x=x*2
         call TLS_AMPI_Send(x, xd, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, err_code)
         call TLS_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         yd=yd*3
         y=y*3
      ELSE IF (world_rank.EQ.1) THEN
         call TLS_AMPI_IRecv(local, locald, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_FROM_SEND, MPI_COMM_WORLD, req, err_code)
         call TLS_AMPI_Wait(req, MPI_STATUS_IGNORE, err_code)
         locald = COS(local)*locald
         local = SIN(local)
         call TLS_AMPI_Send(local, locald, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xd,yd

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         xd=1.0
         x=2.5
         print *,"process",world_rank," sends val  [ 2.500000000==]",x
         call head_d(x,xd,y,yd)
         print *,"process",world_rank," recvs val  [-2.876772823==]",y
         print *,"process",world_rank," recvs diff [1.7019731128==]",yd
         yd=yd+xd
         y=y+x
      ELSE IF (world_rank.EQ.1) THEN
         call head_d(x,xd,y,yd)
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

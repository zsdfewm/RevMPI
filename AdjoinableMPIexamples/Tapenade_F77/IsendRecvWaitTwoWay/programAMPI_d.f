c   DESIRED TAPENADE TANGENT USING THE AMPI LIBRARY
c Simple Isend+Recv+Wait || Recv+Isend+Wait

      SUBROUTINE head_d(x,xd,y,yd)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION xd,yd
      DOUBLE PRECISION x,y
      INTEGER world_rank, err_code, req
      DOUBLE PRECISION locald
      DOUBLE PRECISION local

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         xd=xd*2
         x=x*2
         call TLS_AMPI_ISend(x, xd, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_TO_RECV, MPI_COMM_WORLD, req, err_code)
         call TLS_AMPI_Recv(y, yd, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD,
     +        MPI_STATUS_IGNORE, err_code)
         call TLS_AMPI_Wait(req,MPI_STATUS_IGNORE, err_code)
         yd=yd*3
         y=y*3
      ELSE IF (world_rank.EQ.1) THEN
         call TLS_AMPI_Recv(local, locald, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD,
     +        MPI_STATUS_IGNORE, err_code)
         locald = COS(local)*locald
         local = SIN(local)
         call TLS_AMPI_ISend(local, locald, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_TO_RECV, MPI_COMM_WORLD, req, err_code)
         call TLS_AMPI_Wait(req,MPI_STATUS_IGNORE, err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION xd,yd
      DOUBLE PRECISION x,y

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         xd=1.0
         x=3.5
         print *,"process",world_rank," sends val  [3.50000000==]",x
         call head_d(x,xd,y,yd)
         print *,"process",world_rank," recvs val  [1.97095979==]",y
         print *,"process",world_rank," recvs diff [4.52341353==]",yd
         y=y+x
      ELSE IF (world_rank.EQ.1) THEN
         call head_d(x,xd,y,yd)
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

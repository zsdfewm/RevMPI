c   DESIRED TAPENADE TANGENT USING THE AMPI LIBRARY
c   One proc just sends, one proc just receives

      SUBROUTINE HEAD_D(v1, v1d, v2, v2d)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION v1, v2
      DOUBLE PRECISION v1d, v2d
      INTEGER world_rank, err_code
      DOUBLE PRECISION value
      DOUBLE PRECISION valued

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         valued = 0.5*v1d/SQRT(v1)
         value = SQRT(v1)
         call TLS_AMPI_Send(value, valued, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_TO_RECV, MPI_COMM_WORLD, err_code)
      ELSE IF (world_rank.EQ.1) THEN
         call TLS_AMPI_Recv(value, valued, 1,
     +        AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_FROM_SEND,MPI_COMM_WORLD,MPI_STATUS_IGNORE,err_code)
         v2d = COS(value)*valued
         v2 = SIN(value)
      ENDIF
      END

      PROGRAM MAIN
      implicit none
      include 'ampi/ampif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xd,yd

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         xd=1.0
         x=3.5
         print *,"process",world_rank," sets val  [3.50000000==]",x
         call HEAD_D(x,xd,y,yd)
      ELSE IF (world_rank.EQ.1) THEN
         call HEAD_D(x,xd,y,yd)
         print *,"process",world_rank," gets val  [0.95532692==]",y
         print *,"process",world_rank," gets diff [-7.898936E-002==]",yd
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

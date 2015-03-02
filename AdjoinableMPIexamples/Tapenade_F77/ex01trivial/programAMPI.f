C Hand-turned into AMPI. Not differentiated.
c   One proc just sends, one proc just receives

      SUBROUTINE head(v1, v2)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION v1, v2
      INTEGER world_rank, err_code
      DOUBLE PRECISION value

      call AMPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         value = SQRT(v1)
         call AMPI_Send(value, 1, AMPI_ADOUBLE_PRECISION, 1, 0,
     +        AMPI_TO_RECV, MPI_COMM_WORLD, err_code)
      ELSE IF (world_rank.EQ.1) THEN
         call AMPI_Recv(value, 1, AMPI_ADOUBLE_PRECISION, 0, 0,
     +        AMPI_FROM_SEND, MPI_COMM_WORLD,
     +        MPI_STATUS_IGNORE, err_code)
         v2 = SIN(value)
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
         x=3.5
         print *,"process",world_rank," sets val  [3.50000000==]",x
         call head(x,y)
      ELSE IF (world_rank.EQ.1) THEN
         call head(x,y)
         print *,"process",world_rank," gets val  [0.95532692==]",y
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

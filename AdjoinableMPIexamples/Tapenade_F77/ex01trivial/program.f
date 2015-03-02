c   One proc just sends, one proc just receives

      SUBROUTINE head(v1, v2)
      implicit none
      include 'mpif.h'
      DOUBLE PRECISION v1, v2
      INTEGER world_rank, err_code
      DOUBLE PRECISION value

      call MPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         value = SQRT(v1)
         call MPI_Send(value, 1, MPI_DOUBLE_PRECISION, 1, 0,
     +        MPI_COMM_WORLD, err_code)
      ELSE IF (world_rank.EQ.1) THEN
         call MPI_Recv(value, 1, MPI_DOUBLE_PRECISION, 0, 0,
     +        MPI_COMM_WORLD, MPI_STATUS_IGNORE, err_code)
         v2 = SIN(value)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'mpif.h'
      INTEGER world_rank, err_code
      DOUBLE PRECISION x,y

      call MPI_Init(err_code)
      call MPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         x=3.5
         print *,"process",world_rank," sets val  [3.50000000==]",x
         call head(x,y)
      ELSE IF (world_rank.EQ.1) THEN
         call head(x,y)
         print *,"process",world_rank," gets val  [0.95532692==]",y
      ENDIF
      call MPI_Finalize(err_code)
      END

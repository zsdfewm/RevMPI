c    Simple Irecv+Send+Wait || Irecv+Wait+Send

      SUBROUTINE head(x,y)
      implicit none
      include 'mpif.h'
      DOUBLE PRECISION x,y
      INTEGER world_rank, err_code, req
      DOUBLE PRECISION local

      call MPI_Comm_rank(MPI_COMM_WORLD, world_rank, err_code)
      IF (world_rank.EQ.0) THEN
         call MPI_IRecv(y, 1, MPI_DOUBLE_PRECISION, 1, 0,
     +        MPI_COMM_WORLD, req, err_code)
         x=x*2
         call MPI_Send(x, 1, MPI_DOUBLE_PRECISION, 1, 0,
     +        MPI_COMM_WORLD, err_code)
         call MPI_Wait(req,MPI_STATUS_IGNORE, err_code)
         y=y*3
      ELSE IF (world_rank.EQ.1) THEN
         call MPI_IRecv(local, 1, MPI_DOUBLE_PRECISION, 0, 0,
     +        MPI_COMM_WORLD, req, err_code)
         call MPI_Wait(req,MPI_STATUS_IGNORE, err_code)
         local = SIN(local)
         call MPI_Send(local, 1, MPI_DOUBLE_PRECISION, 0, 0,
     +        MPI_COMM_WORLD, err_code)
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
         x=2.5
         print *,"process",world_rank," sends val  [ 2.500000000==]",x
         call head(x,y)
         print *,"process",world_rank," recvs val  [-2.876772823==]",y
         y=y+x
      ELSE IF (world_rank.EQ.1) THEN
         call head(x,y)
      ENDIF
      call MPI_Finalize(err_code)
      END

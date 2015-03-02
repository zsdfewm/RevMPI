c    Several Send's and a loop of wildcard Recv's

      SUBROUTINE head(x,y)
      implicit none
      include 'mpif.h'
      DOUBLE PRECISION x,y
      INTEGER rank, err_code, req, nbProc
      DOUBLE PRECISION recvx, recvy
      INTEGER status(MPI_STATUS_SIZE)
      INTEGER i

      call MPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      call MPI_Comm_size(MPI_COMM_WORLD, nbProc, err_code)
      IF (rank.EQ.0) THEN
         DO i=1,nbProc-1
            call MPI_Recv(recvy, 1, MPI_DOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,12, MPI_COMM_WORLD,status, err_code)
            y = y*recvy
            call MPI_Recv(recvx, 1, MPI_DOUBLE_PRECISION,
     +           MPI_ANY_SOURCE, 11, MPI_COMM_WORLD,status, err_code)
            x = x+recvx
         ENDDO
      ELSE
         call MPI_Isend(x,1,MPI_DOUBLE_PRECISION,0,11,
     +        MPI_COMM_WORLD,req,err_code)
         call MPI_Send(y,1,MPI_DOUBLE_PRECISION,0,12,
     +        MPI_COMM_WORLD,err_code)
         call MPI_Wait(req,status,err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'mpif.h'
      INTEGER rank, err_code
      DOUBLE PRECISION x,y

      call MPI_Init(err_code)
      call MPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      IF (rank.EQ.0) THEN
         x = 1.0
         y = 2.0
      ELSE
         x = 1.0+rank
         y = rank/2.0
      ENDIF
      call head(x,y)
      IF (rank.EQ.0) THEN
         print *, 'rank 0: cumulx=',x
         print *, 'rank 0: cumuly=',y
      ENDIF
      call MPI_Finalize(err_code)
      END

c    Several Send's and a loop of wildcard Recv's
c    Modified by hand for AMPI. No differentiation yet.

      SUBROUTINE head(x,y)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x,y
      INTEGER rank, err_code, req, nbProc
      DOUBLE PRECISION recvx, recvy
      INTEGER status(MPI_STATUS_SIZE)
      INTEGER i

      call AMPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      call MPI_Comm_size(MPI_COMM_WORLD, nbProc, err_code)
      IF (rank.EQ.0) THEN
         DO i=1,nbProc-1
            call AMPI_Recv(recvy, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,12,AMPI_FROM_SEND,MPI_COMM_WORLD,
     +           status,err_code)
            y = y*recvy
            call AMPI_Recv(recvx, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,11,AMPI_FROM_ISEND_WAIT,MPI_COMM_WORLD,
     +           status,err_code)
            x = x+recvx
         ENDDO
      ELSE
         call AMPI_Isend(x,1,AMPI_ADOUBLE_PRECISION,0,11,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,req,err_code)
         call AMPI_Send(y,1,AMPI_ADOUBLE_PRECISION,0,12,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,err_code)
         call AMPI_Wait(req,status,err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER rank, err_code
      DOUBLE PRECISION x,y

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
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
      call AMPI_Finalize_NT(err_code)
      END

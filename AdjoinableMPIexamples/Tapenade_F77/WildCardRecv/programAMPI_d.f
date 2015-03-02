c    Several Send's and a loop of wildcard Recv's
c    Tangent differentiation

      SUBROUTINE head_d(x,xd,y,yd)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xd,yd
      INTEGER rank, err_code, req, nbProc
      DOUBLE PRECISION recvx, recvy
      DOUBLE PRECISION recvxd, recvyd
      INTEGER status(MPI_STATUS_SIZE)
      INTEGER i

      call AMPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      call MPI_Comm_size(MPI_COMM_WORLD, nbProc, err_code)
      IF (rank.EQ.0) THEN
         DO i=1,nbProc-1
            call TLS_AMPI_Recv(recvy, recvyd, 1,
     +           AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,12,AMPI_FROM_SEND,MPI_COMM_WORLD,
     +           status,err_code)
            yd = yd*recvy + y*recvyd
            y = y*recvy
            call TLS_AMPI_Recv(recvx, recvxd, 1,
     +           AMPI_ADOUBLE_PRECISION, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,11,AMPI_FROM_ISEND_WAIT,MPI_COMM_WORLD,
     +           status,err_code)
            xd = xd+recvxd
            x = x+recvx
         ENDDO
      ELSE
         call TLS_AMPI_Isend(x,xd,1,
     +        AMPI_ADOUBLE_PRECISION,AMPI_ADOUBLE_PRECISION,0,11,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,req,err_code)
         call TLS_AMPI_Send(y,yd,1,
     +        AMPI_ADOUBLE_PRECISION,AMPI_ADOUBLE_PRECISION,0,12,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,err_code)
         call TLS_AMPI_Wait(req,status,err_code)
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER rank, err_code
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xd,yd
      DOUBLE PRECISION tmpsumdiff,globalsumdiff

      call AMPI_Init_NT(err_code)
      call AMPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      IF (rank.EQ.0) THEN
         x = 1.0
         y = 2.0
      ELSE
         x = 1.0+rank
         y = rank/2.0
      ENDIF
      xd = 1.0
      yd = 1.0
      call head_d(x,xd,y,yd)
      IF (rank.EQ.0) THEN
         print *, 'rank 0: cumul x=',x
         print *, 'rank 0: cumul y=',y
      ENDIF
      globalsumdiff = 0.0
      call MPI_REDUCE(xd,tmpsumdiff,1,MPI_DOUBLE_PRECISION,MPI_SUM,
     +     0,MPI_COMM_WORLD,err_code)
      globalsumdiff = globalsumdiff+tmpsumdiff
      call MPI_REDUCE(yd,tmpsumdiff,1,MPI_DOUBLE_PRECISION,MPI_SUM,
     +     0,MPI_COMM_WORLD,err_code)
      globalsumdiff = globalsumdiff+tmpsumdiff
      IF (rank.EQ.0) THEN
         print *, 'rank 0: sum of all output tangents [27.000==]',
     +        globalsumdiff
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

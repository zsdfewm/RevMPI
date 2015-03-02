c    Several Send's and a loop of wildcard Recv's
c    ADJOINT differentiation

      SUBROUTINE head_b(x,xb,y,yb)
      implicit none
      include 'ampi/ampif.h'
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xb,yb
      INTEGER rank, err_code, req, nbProc
      DOUBLE PRECISION recvx, recvy
      DOUBLE PRECISION recvxb, recvyb
      INTEGER status(MPI_STATUS_SIZE)
      INTEGER i

      call AMPI_Comm_rank(MPI_COMM_WORLD, rank, err_code)
      call MPI_Comm_size(MPI_COMM_WORLD, nbProc, err_code)
      IF (rank.EQ.0) THEN
         DO i=1,nbProc-1
            call pushreal8(recvy)
            call FW_AMPI_Recv(recvy, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,12,AMPI_FROM_SEND,MPI_COMM_WORLD,
     +           status,err_code)
            call pushreal8(y)
            y = y*recvy
            call FW_AMPI_Recv(recvx, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,11,AMPI_FROM_ISEND_WAIT,MPI_COMM_WORLD,
     +           status,err_code)
            x = x+recvx
         ENDDO
         call ADTOOL_AMPI_Turn(x,xb)
         call ADTOOL_AMPI_Turn(y,yb)
         call ADTOOL_AMPI_Turn(recvx,recvxb)
         call ADTOOL_AMPI_Turn(recvy,recvyb)
         recvxb = 0.0
         recvyb = 0.0
         DO i=nbProc-1,1,-1
            recvxb = recvxb+xb
            call BW_AMPI_Recv(recvxb, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,11,AMPI_FROM_ISEND_WAIT,MPI_COMM_WORLD,
     +           status,err_code)
            call popreal8(y)
            recvyb = recvyb + y*yb
            yb = recvy*yb
            call popreal8(recvy)
            call BW_AMPI_Recv(recvyb, 1, AMPI_ADOUBLE_PRECISION,
     +           MPI_ANY_SOURCE,12,AMPI_FROM_SEND,MPI_COMM_WORLD,
     +           status,err_code)
         ENDDO
      ELSE
         call FW_AMPI_Isend(x,1,
     +        AMPI_ADOUBLE_PRECISION,0,11,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,req,err_code)
         call FW_AMPI_Send(y,1,
     +        AMPI_ADOUBLE_PRECISION,0,12,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,err_code)
         call FW_AMPI_Wait(req,status,err_code)
         call ADTOOL_AMPI_Turn(x,xb)
         call ADTOOL_AMPI_Turn(y,yb)
         call BW_AMPI_Wait(req,status,err_code)
         call BW_AMPI_Send(yb,1,
     +        AMPI_ADOUBLE_PRECISION,0,12,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,err_code)
         call BW_AMPI_Isend(xb,1,
     +        AMPI_ADOUBLE_PRECISION,0,11,
     +        AMPI_TO_RECV,MPI_COMM_WORLD,req,err_code)         
      ENDIF
      END

      PROGRAM main
      implicit none
      include 'ampi/ampif.h'
      INTEGER rank, err_code
      DOUBLE PRECISION x,y
      DOUBLE PRECISION xb,yb
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
      xb = 1.0
      yb = 1.0
      call head_b(x,xb,y,yb)
      globalsumdiff = 0.0
      call MPI_REDUCE(xb,tmpsumdiff,1,MPI_DOUBLE_PRECISION,MPI_SUM,
     +     0,MPI_COMM_WORLD,err_code)
      globalsumdiff = globalsumdiff+tmpsumdiff
      call MPI_REDUCE(yb,tmpsumdiff,1,MPI_DOUBLE_PRECISION,MPI_SUM,
     +     0,MPI_COMM_WORLD,err_code)
      globalsumdiff = globalsumdiff+tmpsumdiff
      IF (rank.EQ.0) THEN
         print *, 'rank 0: sum of all input gradients [27.000==]',
     +        globalsumdiff
      ENDIF
      call AMPI_Finalize_NT(err_code)
      END

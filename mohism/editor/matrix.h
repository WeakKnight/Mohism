#pragma once
#define N 50

double getA(double arcs[N][N],int n)
{
    if(n==1)
    {
        return arcs[0][0];
    }
    double ans = 0;
    double temp[N][N]={0.0};
    int i,j,k;
    for(i=0;i<n;i++)
    {
        for(j=0;j<n-1;j++)
        {
            for(k=0;k<n-1;k++)
            {
                temp[j][k] = arcs[j+1][(k>=i)?k+1:k];
                
            }
        }
        double t = getA(temp,n-1);
        if(i%2==0)
        {
            ans += arcs[0][i]*t;
        }
        else
        {
            ans -=  arcs[0][i]*t;
        }
    }
    return ans;
}

void getAStar(double arcs[N][N],int n,double ans[N][N])
{
    if(n==1)
    {
        ans[0][0] = 1;
        return;
    }
    int i,j,k,t;
    double temp[N][N];
    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
        {
            for(k=0;k<n-1;k++)
            {
                for(t=0;t<n-1;t++)
                {
                    temp[k][t] = arcs[k>=i?k+1:k][t>=j?t+1:t];
                }
            }
            
            
            ans[j][i]  =  getA(temp,n-1);
            if((i+j)%2 == 1)
            {
                ans[j][i] = - ans[j][i];
            }
        }
    }
}

void MatrixMultiply(double matrix_a[N][N],double matrix_b[N][N], double matrix_result[N][N], int size_m, int size_n, int size_s)
{
    for(int m = 0; m < size_m; m++)
    {
        for(int s=0; s < size_s; s++)
        {
            matrix_result[m][s] = 0;
            
            for(int n = 0; n < size_n; n++)
            {
                matrix_result[m][s] += matrix_a[m][n] * matrix_b[n][s];
            }
        }
    }
}

bool GetMatrixInverse(double src[N][N], int n, double des[N][N])
{
    double flag=getA(src,n);
    double t[N][N];
    if(flag==0)
    {
        return false;
    }
    else
    {
        getAStar(src,n,t);
        for(int i=0;i<n;i++)
        {
            for(int j=0;j<n;j++)
            {
                des[i][j]=t[i][j]/flag;
            }
            
        }
    }
    
    
    return true;
    
}


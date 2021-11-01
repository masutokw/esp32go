#include <math.h>
#include <string.h>
#include"taki.h"
typedef c_double MAT3x3 [3] [3];
typedef c_double VECT3 [3];
char buffer1[100];
static MAT3x3
eq_trans_mat, //to equatorial transform matrix
altz_trans_mat, //to alt-az transform matrix
y,              //telescope
x;              //actual


static VECT3 vector ;
int align_star_index;
static c_double z1, z2, z3; //Kalman Coeffs
static char perfect_mount;

void reset_transforms(c_double az1,c_double az2,c_double az3)
{
    align_star_index = 0;//-1;
    z1 = az1;
    z2 = az2;
    z3 = az3;
    perfect_mount= (z1==0.0)&&(z2==0.0);
}



void spher_to_rect(c_double az,c_double alt)
{
    az=M_2PI-az;
    alt+=z3;
    c_double cos_az=cos(az);
    c_double cos_alt=cos(alt);
    c_double sin_az=sin(az);
    c_double sin_alt=sin(alt);

    vector[0] = cos_az * cos_alt;
    vector[1] = sin_az * cos_alt;
    vector[2] = sin_alt;

    if (!perfect_mount)
    {
        vector[0] +=  sin_az * cos_alt * z1 - sin_az * z2;
        vector[1] +=  cos_az * z2  - cos_az * sin_alt * z1;
    }
}


void sub_y(c_double f,c_double h)
{
    c_double cosF=cos(f);
    c_double cosH=cos(h);
    c_double sinF=sin(f);
    c_double sinH=sin(h);
    y[0][0] = cosF* cosH ;
    y[1][0] = sinF * cosH ;
    y[2][0] = sinH;

    if (!perfect_mount)
    {
        y[0][0] +=  sinF * z2  - sinF * cosH * z1;
        y[1][0] +=  cosF * sinH * z1 - cosF * z2 ;
    }
}


c_double det_3x3(MAT3x3 *mat)

{
    c_double w;
    w =   (*mat)[0][0] * (*mat)[1][1] * (*mat)[2][2] + (*mat)[0][1] * (*mat)[1][2] * (*mat)[2][0];
    w +=  (*mat)[0][2] * (*mat)[2][1] * (*mat)[1][0];
    w += -(*mat)[0][2] * (*mat)[1][1] * (*mat)[2][0] - (*mat)[0][0] * (*mat)[2][1] * (*mat)[1][2];
    w -=  (*mat)[0][1] * (*mat)[1][0] * (*mat)[2][2];
    return w;
}


void rect_to_polar(c_double *f,c_double *h)
{
    c_double c;

    c = sqrt(y[0][0] * y[0][0] + y[1][0] * y[1][0]);
    *h =  atan2(y[2][0],c);
    *f = atan2(y[1][0] , y[0][0]);
    //*f=(c_double)(*f-(int)(*f/360.0)*360.0);
}

void init_star(unsigned char index, const c_star *star)
{
    align_star_index=index--;
    c_double ra=star->ra - (KTIME*star->timer_count) ;
    c_double cosdec=cos(star->dec);
    x[0][index] = cos(ra) * cosdec;
    x[1][index] = cosdec * sin(ra);
    x[2][index] = sin(star->dec);


    spher_to_rect(star->az,star->alt);
    y[0][index] = vector[0];
    y[1][index] = vector[1];
    y[2][index] = vector[2];
}
void third_star(void)
{
    c_double vect_mod;
    unsigned char i;
    x[0][2] = x[1][0] * x[2][1] - x[2][0] * x[1][1];
    x[1][2] = x[2][0] * x[0][1] - x[0][0] * x[2][1];
    x[2][2] = x[0][0] * x[1][1] - x[1][0] * x[0][1];
    vect_mod = sqrt(x[0][2] * x[0][2] + x[1][2] * x[1][2] + x[2][2] * x[2][2]);

    for (i = 0; i <= 2; i++)
        x[i][2] /= vect_mod;
    y[0][2] = y[1][0] * y[2][1] - y[2][0] * y[1][1];
    y[1][2] = y[2][0] * y[0][1] - y[0][0] * y[2][1];
    y[2][2] = y[0][0] * y[1][1] - y[1][0] * y[0][1];
    vect_mod = sqrt(y[0][2] * y[0][2] + y[1][2] * y[1][2] + y[2][2] * y[2][2]);

    for (i = 0; i <= 2; i++)
        y[i][2] /= vect_mod;
}
void compute_trasform(c_star *star1,c_star *star2)

{
    MAT3x3 aux;
    c_double  e, w;
    unsigned char i, j, m, n, k;

    memset(x, 0, sizeof(x)); //clear matrix arrays
    memset(y, 0, sizeof(y));
    memset(eq_trans_mat, 0, sizeof(eq_trans_mat));
    memset(altz_trans_mat, 0, sizeof(altz_trans_mat));

    init_star(1,star1);
    init_star(2,star2);
    if (align_star_index ==2)
        third_star();

    memcpy(aux,x,sizeof(aux));
    e=det_3x3(&aux);

    for (m = 0; m <= 2; m++)
    {
        memcpy(aux,x,sizeof(aux));

        for (n = 0; n <= 2; n++)
        {
            aux[0][m] = 0.0;
            aux[1][m] = 0.0;
            aux[2][m] = 0.0;
            aux[n][m] = 1.0;
            w=det_3x3(&aux);
            eq_trans_mat[m][n] = w / e;
        }
    }
    memset(altz_trans_mat,0,sizeof(altz_trans_mat));
    for (i = 0; i <= 2; i++)
    {
        for (j = 0; j <= 2; j++)
        {
            for (k = 0; k <= 2; k++)
                altz_trans_mat[i][j] += y[i][k] * eq_trans_mat[k][j];
        }
    }
    for (m = 0; m <= 2; m++)
    {
        memcpy(aux,altz_trans_mat,sizeof(aux));
        e=det_3x3(&aux);

        for (n = 0; n <= 2; n++)
        {
            aux[0][m] = 0.0;
            aux[1][m] = 0.0;
            aux[2][m] = 0.0;
            aux[n][m] = 1.0;
            w=det_3x3(&aux);
            eq_trans_mat[m][n] = w / e;
        }
    }
}

/*
void to_alt_az(c_star *star)

{
    unsigned char i, j;


    c_double tmp0 = star->ra - (KTIME  * star->timer_count) ;
    c_double tmp1=cos(star->dec);
    x[0][0] = tmp1 * cos(tmp0);
    x[1][0] = tmp1 * sin(tmp0);
    x[2][0] = sin(star->dec);
    y[0][0] = 0.0;
    y[1][0] = 0.0;
    y[2][0] = 0.0;
    for (i = 0; i <= 2; i++)
    {
        for (j = 0; j <= 2; j++)
            y[i][0] += altz_trans_mat[i][j] * x[j][0];
    }
    rect_to_polar(&tmp0,&tmp1);
    sub_y(tmp0,tmp1);
    rect_to_polar(&tmp0,&tmp1);
    star->alt =tmp1- z3;
    star->az=M_2PI - tmp0;
    while (star->az<0.0)  star->az+= M_2PI ;
    while (star->az>(M_2PI)) star->az-= M_2PI ;
}
*/
void to_alt_az(c_star *star)
{
    unsigned char i, j;


    c_double tmp0 = star->ra - (KTIME  * star->timer_count) ;
    c_double tmp1=cos(star->dec);
    x[0][0] = tmp1 * cos(tmp0);
    x[1][0] = tmp1 * sin(tmp0);
    x[2][0] = sin(star->dec);
    y[0][0] = 0.0;
    y[1][0] = 0.0;
    y[2][0] = 0.0;
    for (i = 0; i <= 2; i++)
    {
        for (j = 0; j <= 2; j++)
            y[i][0] += altz_trans_mat[i][j] * x[j][0];
    }
    rect_to_polar(&tmp0,&tmp1);
    sub_y(tmp0,tmp1);
    rect_to_polar(&tmp0,&tmp1);
    star->alt =tmp1- z3;
    star->az=M_2PI - tmp0;
    if (star->p_mode){
     star->alt=M_PI-star->alt;
     star->az+=M_PI;
     }
    while (star->az<0.0)  star->az+= M_2PI ;
    while (star->az>(M_2PI)) star->az-= M_2PI ;
    
}

void to_equatorial(c_star *star)
{
    unsigned char i, j;
    //  c_double ra_tmp;
    spher_to_rect(star->az, star->alt);
    x[0][0] = vector[0];
    x[1][0] = vector[1];
    x[2][0] = vector[2];
    y[0][0] = 0.0;
    y[1][0] = 0.0;
    y[2][0] = 0.0;
    for (i = 0; i <= 2; i++)
    {
        for (j = 0; j <= 2; j++)
            y[i][0] += eq_trans_mat[i][j] * x[j][0];
    }
    rect_to_polar(&(star->ra),&(star->dec));
    star->ra+= KTIME  * star->timer_count ;
    while  (star->ra<0.0)   star->ra+= M_2PI;
    while  (star->ra>M_2PI) star->ra-= M_2PI;


}
void set_star(c_star *st,c_double ra,c_double dec,c_double az,c_double alt,c_double ticks)
{
    st->ra=ra/RAD_TO_DEG;
    st->dec=dec/RAD_TO_DEG;
    st->az=az/RAD_TO_DEG;;
    st->alt=alt/RAD_TO_DEG;
    st->timer_count=ticks;

}

void print_matrix (MAT3x3 *mat, const char *txt)
{
    unsigned char i ;
    printf ("%s\r\n", txt) ;
    for (i=0 ; i<3 ; i++)
    {
        printf ("%9.5f  %9.5f %9.5f |\r\n", (*mat)[i][0],(*mat)[i][1],(*mat)[i][2]) ;
    }
    printf( "\r\n") ;
}

void printm(void)
{
    print_matrix(&x,"x");
    print_matrix(&y,"y");
}

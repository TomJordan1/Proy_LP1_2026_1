#include <math.h>

#include "Simulador.h"
void actualizar_fisica_basica(Particula *p, Config c) {
    p->vy += c.g * c.dt;
    p->vx *= (1.0 - c.mu * c.dt);
    p->vy *= (1.0 - c.mu * c.dt);
    p->x += p->vx * c.dt;
    p->y += p->vy * c.dt;
}

void resolver_colision_paredes(Particula *p, Config c) {
    if (p->x < p->r) {
        p->x = p->r;
        p->vx *= -c.e;
    }
    if (p->x > c.ancho - p->r) {
        p->x = c.ancho - p->r;
        p->vx *= -c.e;
    }
    if (p->y < p->r) {
        p->y = p->r;
        p->vy *= -c.e;
    }
    if (p->y > c.alto - p->r) {
        p->y = c.alto - p->r;
        p->vy *= -c.e;
    }
}

void resolver_colision_particulas(Particula *p1, Particula *p2) {
    double dx = p2->x - p1->x;
    double dy = p2->y - p1->y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < p1->r + p2->r && dist > 0.0001) {
        double solapamiento = p1->r + p2->r - dist;
        double nx = dx / dist;
        double ny = dy / dist;

        double masa_total = p1->m + p2->m;
        double m1 = p2->m / masa_total;
        double m2 = p1->m / masa_total;

        p1->x -= solapamiento * nx * m2;
        p1->y -= solapamiento * ny * m2;
        p2->x += solapamiento * nx * m1;
        p2->y += solapamiento * ny * m1;

        double dvx = p2->vx - p1->vx;
        double dvy = p2->vy - p1->vy;
        double dot_product = dvx * nx + dvy * ny;

        if (dot_product < 0) {
            double e = 1.0;
            double impulso = -(1.0 + e) * dot_product;
            impulso /= (1.0 / p1->m + 1.0 / p2->m);

            double ix = impulso * nx;
            double iy = impulso * ny;

            p1->vx -= ix / p1->m;
            p1->vy -= iy / p1->m;
            p2->vx += ix / p2->m;
            p2->vy += iy / p2->m;
        }
    }
}
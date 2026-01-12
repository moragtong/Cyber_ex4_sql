FROM php:apache

COPY index.php /var/www/html
COPY apache2.conf /etc/apache2/apache2.conf
# RUN apt install iproute2
RUN docker-php-ext-install mysqli 

EXPOSE 80

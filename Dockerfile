FROM alpine:latest AS builder
RUN apk add --no-cache build-base
WORKDIR /app
COPY . .
RUN make

FROM alpine:latest
WORKDIR /app
COPY --from=builder /app/htdocs /app/htdocs
COPY --from=builder /app/uhttpd /app/uhttpd
EXPOSE 8080
CMD ["./uhttpd"]
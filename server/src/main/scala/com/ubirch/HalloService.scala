package com.ubirch

import java.io.{File, FileOutputStream}

import akka.actor.Actor
import spray.http._
import spray.routing._

class HalloServiceActor extends Actor with HalloService {
  def actorRefFactory = context

  def receive = runRoute(routes)
}

trait HalloService extends HttpService {

  val routes =
    path("") {
      get {
        complete("HALLO")
      }
    } ~
      path("""\d+""".r) { id =>
        detach() {
          get {
            getFromFile(s"upload_$id.ogg")
          } ~
          post {
            entity(as[Array[Byte]]) { value =>
              val byteWriter = new FileOutputStream(new File(s"upload_$id.ogg"))
              byteWriter.write(value)
              byteWriter.flush()
              byteWriter.close()
              println(s"RECEIVED FILE: $id")
              complete(s"$id")
            }
          }
        }
      }
}
package com.ubirch

import java.io.{File, FileOutputStream}
import java.text.SimpleDateFormat
import java.util.{Calendar, TimeZone}

import akka.actor.Actor
import spray.http.HttpResponse
import spray.routing._

import scala.util.Try

case class CleanupFile(file: Option[File])

class HalloServiceActor extends Actor with HalloService {
  def actorRefFactory = context

  def receive = runRoute(routes) orElse {
    case CleanupFile(file) => file.foreach(f => Try(f.delete))
  }

}

trait HalloService extends HttpService {
  val dataDir = new File("data")
  var connections: Map[String, String] = Map(
    //    "866224021719199" -> "866224021719199"
  )

  if (!dataDir.exists()) {
    dataDir.mkdirs()
  }

  val routes =
    path("") {
      get {
        complete("HALLO")
      }
    } ~
      path( """\d+""".r) { id =>
        detach() {
          get {
            if (!dataDir.exists()) reject()

            // try to find the newest file available
            val file = dataDir.listFiles
              .filter(f => f.getName.matches(s"message_${id}_\\d+\\.ogg"))
              .sortBy(_.getName).headOption

            // delete file after successful delivery
            mapRequestContext { ctx =>
              ctx.withRouteResponseMapped {
                case r: HttpResponse => r.withAck(CleanupFile(file))
                case x => x
              }
            } {
              file match {
                case Some(f) => getFromFile(f)
                case None => reject
              }
            }
          }
        } ~ post {
          entity(as[Array[Byte]]) { value =>
            val recipient = connections.getOrElse(id, id)
            val cal = Calendar.getInstance(TimeZone.getTimeZone("UTC"))
            val df = new SimpleDateFormat("yyyyMMddHHmmss")
            val name = s"message_${recipient}_${df.format(cal.getTimeInMillis)}"
            val file = new File(dataDir, s"$name.tmp")
            val byteWriter = new FileOutputStream(file)
            byteWriter.write(value)
            byteWriter.flush()
            byteWriter.close()

            println(s"RECEIVED FILE: $id")
            val target = new File(dataDir, s"$name.ogg")
            if(file.renameTo(target)) {
              complete(s"$id")
            } else {
              reject
            }
          }
        }
      }
}
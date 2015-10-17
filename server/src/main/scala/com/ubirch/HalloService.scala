package com.ubirch

import java.io.{File, FileOutputStream}
import java.text.SimpleDateFormat
import java.util.{Calendar, TimeZone}

import akka.actor.{Actor, ActorLogging}
import akka.util.Timeout
import spray.can.Http
import spray.http._

import scala.concurrent.Future
import scala.concurrent.duration._

case class DeleteFile(file: File)

class HalloServiceActor extends Actor with ActorLogging {
  implicit val timeout: Timeout = 1.second // for the actor 'asks'

  import context.dispatcher

  // ExecutionContext for the futures and scheduler

  final val dataDir = new File("data")
  var connections: Map[String, String] = Map(
    "866224021723183" -> "866224021727127",
    "866224021727127" -> "866224021723183"
  )

  def receive = {
    case _: Http.Connected => sender ! Http.Register(self)

    case HttpRequest(HttpMethods.GET, Uri.Path("/"), _, _, _) =>
      log.info("HALLO")
      sender ! HttpResponse(entity = "HALLO")

    case HttpRequest(HttpMethods.GET, Uri.Path(pathId), _, _, _) if pathId.matches("/\\d+") =>
      val id = pathId.substring(1)
      if (dataDir.exists()) {
        log.info(s"GET: $id")
        // try to find the newest file available
        val file = dataDir.listFiles
          .filter(f => f.getName.matches(s"message_${id}_\\d+\\.ogg"))
          .sortBy(_.getName).headOption
        log.info(s"FOUND: $file")

        file match {
          case Some(f) => sender ! HttpResponse(entity = HttpData(f)).withAck(DeleteFile(f))
          case None => sender ! HttpResponse(status = StatusCodes.NotFound)
        }
      } else {
        sender ! HttpResponse(status = StatusCodes.NotFound)
      }

    case HttpRequest(HttpMethods.POST, Uri.Path(pathId), _, entity: HttpEntity.NonEmpty, _) if pathId.matches("/\\d+") =>
      val id = pathId.substring(1)

      dataDir.mkdir()

      log.info(s"POST $id")
      val recipient = connections.getOrElse(id, id)
      val cal = Calendar.getInstance(TimeZone.getTimeZone("UTC"))
      val df = new SimpleDateFormat("yyyyMMddHHmmss")
      val name = s"message_${recipient}_${df.format(cal.getTimeInMillis)}"

      val file = new File(dataDir, s"$name.tmp")
      val byteWriter = new FileOutputStream(file)
      byteWriter.write(entity.data.toByteArray)
      byteWriter.flush()
      byteWriter.close()
      log.info(s"SAVED ${file.getName}")

      val target = new File(dataDir, s"$name.ogg")
      if (file.renameTo(target)) {
        sender ! HttpResponse(status = StatusCodes.OK)
      } else {
        sender ! HttpResponse(status = StatusCodes.ServiceUnavailable)
      }

    case _: HttpRequest => sender ! HttpResponse(status = StatusCodes.NotFound)


    case DeleteFile(file) =>
      try {
        if (file.delete)
          log.debug(s"deleted $file")
        else
          log.debug(s"unable to delete $file")
      } catch {
        case e: Exception => println(s"exception deleting $file")
      }

  }
}

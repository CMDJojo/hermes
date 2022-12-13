import '../styles/DetailInfo.css';
import { AnimatePresence, motion } from 'framer-motion';
import { DetailInfoData } from './Map';

interface DetailInfoProps {
  info: DetailInfoData | null;
}

interface RouteProps {
  passengerCount: number;
  routeName: string;
  bgColor: string;
  fgColor: string;
  headsign: string;
}

function Route({
  passengerCount,
  routeName,
  bgColor,
  fgColor,
  headsign,
}: RouteProps) {
  return (
    <div className="Route">
      <div
        className="icon"
        style={{ color: fgColor, backgroundColor: bgColor }}
      >
        {routeName}
      </div>
      <div className="headsign">{headsign}</div>
      <div>{passengerCount} personer</div>
    </div>
  );
}

export default function DetailInfo({ info }: DetailInfoProps) {
  const total = info?.map(r => r.passengerCount).reduce((acc, a) => acc + a, 0);

  const routes = info?.map(r => (
    <Route
      key={r.routeName + r.headsign}
      passengerCount={r.passengerCount}
      routeName={r.routeName}
      bgColor={r.bgColor}
      fgColor={r.fgColor}
      headsign={r.headsign}
    />
  ));

  return (
    <AnimatePresence>
      {info !== null && (
        <motion.div
          initial={{ opacity: 0, scale: 0.8 }}
          animate={{ opacity: 1, scale: 1 }}
          exit={{ opacity: 0 }}
          transition={{ duration: 0.1, scale: 0.5 }}
          className="DetailInfo"
        >
          {routes}
          {routes !== undefined && routes.length > 1 && (
            <strong>Totalt {total} personer</strong>
          )}
        </motion.div>
      )}
    </AnimatePresence>
  );
}
